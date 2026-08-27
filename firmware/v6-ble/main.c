/*
 * V6 Geiger transmitter: nRF24L01+ BLE advertisements
 *
 * Hardware: ATtiny2313 + nRF24L01+ + SBM-20 (TransmitterPCB V6)
 * Pinout from the last working firmware (commit bf09a87).
 *
 * Each minute the node broadcasts a non-connectable BLE advertisement
 * with device ID and the pulse count for that window (CPM).
 *
 * High voltage: Timer0 Fast PWM on OC0A/PB2, same as the original
 * avrTx / Greifswald code (OCR0A ~125 was "400 V"). The 555 on the
 * schematic can coexist; PWM is kept because those boards worked with
 * it. PWM must keep running, so the MCU idles rather than powering down.
 *
 * BLE fakery after Dmitry Grinberg (non-commercial use; see README).
 */

#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>

/* ---- board ---- */
#define DEVICE_ID     0x33
#define HV_PWM_OCR    125u      /* original "Set to 400V" */
#define REPORT_SECONDS 60u

#define NRF_DDR_CE    DDRD
#define NRF_PORT_CE   PORTD
#define NRF_CE        PD6
#define NRF_DDR_CSN   DDRD
#define NRF_PORT_CSN  PORTD
#define NRF_CSN       PD5
#define NRF_DDR_SCK   DDRB
#define NRF_PORT_SCK  PORTB
#define NRF_SCK       PB0
#define NRF_DDR_MOSI  DDRD
#define NRF_PORT_MOSI PORTD
#define NRF_MOSI      PD4
#define NRF_DDR_MISO  DDRB
#define NRF_PIN_MISO  PINB
#define NRF_MISO      PB1

#define PULSE_PIN     PD2       /* INT0, active low impulse */

#define MAC0 0xC2
#define MAC1 0xE1
#define MAC2 0x6E
#define MAC3 0x52
#define MAC4 0x00
#define MAC5 DEVICE_ID

/* nRF24 register write addresses (W_REGISTER | reg) */
#define NRF_W_REG     0x20
#define NRF_R_REG     0x00
#define NRF_STATUS    0x07
#define NRF_RF_CH     0x05
#define NRF_CONFIG    0x00
#define NRF_FLUSH_TX  0xE1
#define NRF_FLUSH_RX  0xE2
#define NRF_W_TX_PAYLOAD 0xA0
#define NRF_NOP       0xFF

static volatile uint16_t pulses;
static volatile uint8_t  seconds;
static volatile uint8_t  report_due;

/* ---------- HV pump: Timer0 Fast PWM on PB2 / OC0A ---------- */
static void hv_pwm_init(void)
{
	/* Fast PWM, non-inverting OC0A, clock / 1 -> ~3.91 kHz at 1 MHz */
	TCCR0A = (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
	TCCR0B = (1 << CS00);
	OCR0A  = HV_PWM_OCR;
	DDRB  |= (1 << PB2);
}

/* ---------- 1 Hz timebase: Timer1 CTC, runs in idle ---------- */
static void timer1_1hz_init(void)
{
	/* 1 MHz / 64 = 15625 Hz; OCR1A=15624 -> 1.000 s */
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
	OCR1A  = 15624;
	TIMSK  |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
	wdt_reset();
	if (++seconds >= REPORT_SECONDS) {
		seconds = 0;
		report_due = 1;
	}
}

/* ---------- GM pulse: falling edge INT0 (idle sleep allows edges) ---------- */
static void pulse_init(void)
{
	DDRD  &= ~(1 << PULSE_PIN);
	PORTD &= ~(1 << PULSE_PIN);     /* no pull-up; the collector is driven */
	GIMSK |= (1 << INT0);
	MCUCR  = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (1 << ISC01); /* falling */
}

ISR(INT0_vect)
{
	pulses++;
}

static uint16_t take_pulses(void)
{
	uint16_t n;
	cli();
	n = pulses;
	pulses = 0;
	sei();
	return n;
}

/* ---------- bit-bang SPI to nRF24 ---------- */
static uint8_t spi_rw(uint8_t v)
{
	uint8_t i;
	for (i = 0; i < 8; i++) {
		if (v & 0x80)
			NRF_PORT_MOSI |= (1 << NRF_MOSI);
		else
			NRF_PORT_MOSI &= ~(1 << NRF_MOSI);
		NRF_PORT_SCK |= (1 << NRF_SCK);
		v <<= 1;
		if (NRF_PIN_MISO & (1 << NRF_MISO))
			v |= 1;
		NRF_PORT_SCK &= ~(1 << NRF_SCK);
	}
	return v;
}

static void nrf_csn_lo(void) { NRF_PORT_CSN &= ~(1 << NRF_CSN); }
static void nrf_csn_hi(void) { NRF_PORT_CSN |=  (1 << NRF_CSN); }
static void nrf_ce_lo(void)  { NRF_PORT_CE  &= ~(1 << NRF_CE); }
static void nrf_ce_hi(void)  { NRF_PORT_CE  |=  (1 << NRF_CE); }

static void nrf_cmd(uint8_t cmd, uint8_t data)
{
	nrf_csn_lo();
	spi_rw(cmd);
	spi_rw(data);
	nrf_csn_hi();
}

static void nrf_simple(uint8_t cmd)
{
	nrf_csn_lo();
	spi_rw(cmd);
	nrf_csn_hi();
}

static uint8_t swapbits(uint8_t a)
{
	a = (uint8_t)((a & 0xF0) >> 4 | (a & 0x0F) << 4);
	a = (uint8_t)((a & 0xCC) >> 2 | (a & 0x33) << 2);
	a = (uint8_t)((a & 0xAA) >> 1 | (a & 0x55) << 1);
	return a;
}

/* ---------- BLE CRC24 + whitening (Dmitry Grinberg) ---------- */
static void btLeCrc(const uint8_t *data, uint8_t len, uint8_t *dst)
{
	uint8_t v, t, d;
	while (len--) {
		d = *data++;
		for (v = 0; v < 8; v++, d >>= 1) {
			t = (uint8_t)(dst[0] >> 7);
			dst[0] = (uint8_t)(dst[0] << 1);
			if (dst[1] & 0x80) dst[0] |= 1;
			dst[1] = (uint8_t)(dst[1] << 1);
			if (dst[2] & 0x80) dst[1] |= 1;
			dst[2] = (uint8_t)(dst[2] << 1);
			if (t != (d & 1)) {
				dst[2] ^= 0x5B;
				dst[1] ^= 0x06;
			}
		}
	}
}

static void btLeWhiten(uint8_t *data, uint8_t len, uint8_t coeff)
{
	uint8_t m;
	while (len--) {
		for (m = 1; m; m <<= 1) {
			if (coeff & 0x80) {
				coeff ^= 0x11;
				*data ^= m;
			}
			coeff = (uint8_t)(coeff << 1);
		}
		data++;
	}
}

static void btLePacketEncode(uint8_t *packet, uint8_t len, uint8_t chan)
{
	uint8_t i, dataLen = (uint8_t)(len - 3);
	btLeCrc(packet, dataLen, packet + dataLen);
	for (i = 0; i < 3; i++, dataLen++)
		packet[dataLen] = swapbits(packet[dataLen]);
	btLeWhiten(packet, len, (uint8_t)(swapbits(chan) | 2));
	for (i = 0; i < len; i++)
		packet[i] = swapbits(packet[i]);
}

static void nrf_init_ble(void)
{
	uint8_t buf[5];

	NRF_DDR_CE   |= (1 << NRF_CE);
	NRF_DDR_CSN  |= (1 << NRF_CSN);
	NRF_DDR_SCK  |= (1 << NRF_SCK);
	NRF_DDR_MOSI |= (1 << NRF_MOSI);
	NRF_DDR_MISO &= ~(1 << NRF_MISO);
	nrf_ce_lo();
	nrf_csn_hi();
	NRF_PORT_SCK &= ~(1 << NRF_SCK);

	_delay_ms(2);

	nrf_cmd(NRF_W_REG | NRF_CONFIG, 0x12); /* PWR_UP, PTX, no CRC */
	nrf_cmd(NRF_W_REG | 0x01, 0x00);       /* EN_AA off */
	nrf_cmd(NRF_W_REG | 0x02, 0x00);       /* no RX pipes */
	nrf_cmd(NRF_W_REG | 0x03, 0x02);       /* 4-byte address */
	nrf_cmd(NRF_W_REG | 0x04, 0x00);       /* no retransmit */
	nrf_cmd(NRF_W_REG | 0x06, 0x06);       /* 1 Mbps, 0 dBm */
	nrf_cmd(NRF_W_REG | NRF_STATUS, 0x3E);
	nrf_cmd(NRF_W_REG | 0x1C, 0x00);       /* DYNPD off */
	nrf_cmd(NRF_W_REG | 0x1D, 0x00);       /* FEATURE off */

	/* TX_ADDR = bit-reversed BLE access address 0x8E89BED6 */
	buf[0] = NRF_W_REG | 0x10;
	buf[1] = swapbits(0x8E);
	buf[2] = swapbits(0x89);
	buf[3] = swapbits(0xBE);
	buf[4] = swapbits(0xD6);
	nrf_csn_lo();
	spi_rw(buf[0]); spi_rw(buf[1]); spi_rw(buf[2]); spi_rw(buf[3]); spi_rw(buf[4]);
	nrf_csn_hi();
}

static void nrf_power_down(void)
{
	nrf_ce_lo();
	nrf_cmd(NRF_W_REG | NRF_CONFIG, 0x10); /* PWR_UP=0 */
}

static void nrf_power_up_tx(void)
{
	nrf_cmd(NRF_W_REG | NRF_CONFIG, 0x12);
	_delay_us(150); /* crystal / regulator start */
}

static void nrf_transmit(const uint8_t *pkt, uint8_t len, uint8_t rf_ch)
{
	uint8_t i, st;
	uint16_t spin;

	nrf_cmd(NRF_W_REG | NRF_RF_CH, rf_ch);
	nrf_cmd(NRF_W_REG | NRF_STATUS, 0x6E);
	nrf_simple(NRF_FLUSH_TX);
	nrf_simple(NRF_FLUSH_RX);

	nrf_csn_lo();
	spi_rw(NRF_W_TX_PAYLOAD);
	for (i = 0; i < len; i++)
		spi_rw(pkt[i]);
	nrf_csn_hi();

	nrf_ce_hi();
	_delay_us(20);
	nrf_ce_lo();

	for (spin = 4000; spin; spin--) {
		nrf_csn_lo();
		st = spi_rw(NRF_NOP);
		nrf_csn_hi();
		if (st & 0x30) /* TX_DS or MAX_RT */
			break;
	}
	nrf_cmd(NRF_W_REG | NRF_STATUS, 0x30);
}

/*
 * Advertisement layout (before whitening), manufacturer 0xFFFF:
 *   0x42  ADV_NONCONN_IND, random address
 *   0x14  20 bytes follow (MAC + AD)
 *   MAC[6]
 *   02 01 06              flags, LE-only general discoverable
 *   03 08 'G' 'M'         short name
 *   06 FF FF FF id lo hi  manufacturer: id + CPM little-endian
 *   CRC24 placeholder
 *
 * In nRF Connect: name "GM", mfg 0xFFFF, bytes [id, cpm_lo, cpm_hi].
 */
static void ble_advertise(uint8_t id, uint16_t cpm)
{
	static const uint8_t chRf[3] = {2, 26, 80};
	static const uint8_t chLe[3] = {37, 38, 39};
	uint8_t buf[32];
	uint8_t ch, L;

	nrf_power_up_tx();

	for (ch = 0; ch < 3; ch++) {
		L = 0;
		buf[L++] = 0x42;
		buf[L++] = 20;
		buf[L++] = MAC0;
		buf[L++] = MAC1;
		buf[L++] = MAC2;
		buf[L++] = MAC3;
		buf[L++] = MAC4;
		buf[L++] = MAC5;
		buf[L++] = 2;
		buf[L++] = 0x01;
		buf[L++] = 0x06;
		buf[L++] = 3;
		buf[L++] = 0x08;
		buf[L++] = 'G';
		buf[L++] = 'M';
		buf[L++] = 6;
		buf[L++] = 0xFF;
		buf[L++] = 0xFF;
		buf[L++] = 0xFF;
		buf[L++] = id;
		buf[L++] = (uint8_t)cpm;
		buf[L++] = (uint8_t)(cpm >> 8);
		buf[L++] = 0x55;
		buf[L++] = 0x55;
		buf[L++] = 0x55;

		btLePacketEncode(buf, L, chLe[ch]);
		nrf_transmit(buf, L, chRf[ch]);
	}

	nrf_power_down();
}

static void gpio_quiet(void)
{
	/* Unused pins: inputs with pull-up so they don't float. */
	DDRA  &= (uint8_t)~((1 << PA0) | (1 << PA1));
	PORTA |= (1 << PA0) | (1 << PA1);
	/* PA2 is RESET — leave it alone */
	DDRD  &= (uint8_t)~((1 << PD0) | (1 << PD1) | (1 << PD3));
	PORTD |= (1 << PD0) | (1 << PD1) | (1 << PD3);
	DDRB  &= (uint8_t)~((1 << PB3) | (1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7));
	PORTB |= (1 << PB3) | (1 << PB4) | (1 << PB5) | (1 << PB6) | (1 << PB7);

	ACSR |= (1 << ACD); /* analog comparator off; AIN0/AIN1 are nRF SCK/MISO anyway */
}

int main(void)
{
	wdt_reset();
	wdt_enable(WDTO_2S);

	gpio_quiet();
	hv_pwm_init();
	pulse_init();
	timer1_1hz_init();
	nrf_init_ble();
	nrf_power_down();

	sei();
	set_sleep_mode(SLEEP_MODE_IDLE);

	for (;;) {
		if (report_due) {
			uint16_t cpm;
			report_due = 0;
			cpm = take_pulses();
			ble_advertise(DEVICE_ID, cpm);
			wdt_reset();
		}
		sleep_mode(); /* Timer0 PWM, Timer1, and INT0 keep running */
	}
}

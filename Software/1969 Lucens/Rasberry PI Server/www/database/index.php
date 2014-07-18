<html>
	<head>

	<title>Retrieve data from database </title>

	</head>
	<body>
	
	<?php
	// Connect to database server
	mysql_connect("localhost", "root", "Penetent2401") or die (mysql_error ());

	// Select database
	mysql_select_db("gieger") or die(mysql_error());

	// SQL query
	$strSQL = "SELECT * FROM cpm";

	// Execute the query (the recordset $rs contains the result)
	$rs = mysql_query($strSQL);

	echo "<table border='1'>
	<tr>
	<th>Timestamp</th>
	<th>Transmitter ID</th>
	<th>CPM</th>
	</tr>";

	while($row = mysql_fetch_array($rs))
  	{
	  	echo "<tr>";
	  	echo "<td>" . $row['ts'] . "</td>";
	  	echo "<td>" . $row['transmitter'] . "</td>";
	  	echo "<td>" . $row['counts'] . "</td>";
  		echo "</tr>";
  	}
	echo "</table>";

	// Close the database connection
	mysql_close();
	?>

	</body>
</html>
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
	
	// Loop the recordset $rs
	while($row = mysql_fetch_array($rs)) {

	   // Write the value of the column FirstName and BirthDate
	  echo $row['ts'] . " " . $row['counts'] . "<br />";

	  }

	// Close the database connection
	mysql_close();
	?>

	</body>
</html>
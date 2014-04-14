<html>
 <head>
  <title>PHP Test</title>
 </head>
 <body>  
  <?php
		echo '<p>Hello World</p>';
        $connect=mysql_connect("localhost","root","Penetent2401") or die("Unable to Connect");
        mysql_select_db("gieger") or die("Could not open the db");
        $showtablequery="SHOW TABLES FROM gieger";
        $query_result=mysql_query($showtablequery);
        while($showtablerow = mysql_fetch_array($query_result))
        {
        echo $showtablerow[0]." ";
        } 
  ?>
 </body>
</html>
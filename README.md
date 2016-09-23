# Network_magnetometer
Creating a networked magnetometer using the Micro Mag 3, Arduino and Ethernet Shield.

It will take 90 readings of each axis each min, average, remove issues, and post to URL.

Here is a basic example of how the data is submitted: you will need to build your own DB_insert function with password and the like.

<?php
	
	$debug = false;
	$mags = Null;
	
	if ( isset($_GET['mags']) ) {
		$mags=$_GET["mags"];
	} else 
	if ( isset($_POST['mags']) ) {
		$mags=$_POST["mags"];
	}
	if ($mags) {
		$mag=explode(',',$mags);
		$timestamp = date('Y-m-d H:i');
		$sql_ins = "INSERT INTO TABLE_NAMEVALUES ('".$timestamp.":00 UTC','".$mag[0]."','".$mag[1]."','".$mag[2]."','".$mag[3]."' );";		
		DB_insert($sql_ins);
		if ($debug) echo "<br>".$sql_ins;
	}
	else if ($debug) echo "<br>Nothing passed";
?>


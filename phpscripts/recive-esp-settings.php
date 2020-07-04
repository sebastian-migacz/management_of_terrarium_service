<?php
$servername = "localhost";
$dbname = "id13473022_espsmartterradb";
$username = "id13473022_smartadmin";
$password = "Smart_pass123";
$separator = "x";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT ID, SPRINKLER_STATUS, SPRINKLER_HUM, WATERFALL_STATUS, WATERFALL_ON_TIME, WATERFALL_OFF_TIME, GROWLIGHT_STATUS, GROWLIGHT_ON_TIME, GROWLIGHT_OFF_TIME, RGB_R, RGB_G, RGB_B, EVENT_SUNRISE, SUNRISE_TIME, EVENT_SUNSET, SUNSET_TIME, EVENT_RAIN, RAIN_FREQUENCY FROM SettingsData where id=1";

if ($result = $conn->query($sql)) {
    
		
		while ($row = $result->fetch_assoc()) {
	    	echo $row['ID'];
	    	echo $separator;
	    	echo $row['SPRINKLER_STATUS'];
	    	echo $separator;
	    	echo $row['SPRINKLER_HUM'];
	    	echo $separator;
	    	echo $row['WATERFALL_STATUS'];
	    	echo $separator;
	    	echo $row['WATERFALL_ON_TIME'];
	    	echo $separator;
	    	echo $row['WATERFALL_OFF_TIME'];
	    	echo $separator;
	    	echo $row['GROWLIGHT_STATUS'];
	    	echo $separator;
	    	echo $row['GROWLIGHT_ON_TIME'];
	    	echo $separator;
	    	echo $row['GROWLIGHT_OFF_TIME'];
	    	echo $separator;
			echo $row['RGB_R'];
	    	echo $separator;
			echo $row['RGB_G'];
	    	echo $separator;
			echo $row['RGB_B'];
	    	echo $separator;
			echo $row['EVENT_SUNRISE'];
	    	echo $separator;
			echo $row['SUNRISE_TIME'];
	    	echo $separator;
			echo $row['EVENT_SUNSET'];
	    	echo $separator;
			echo $row['SUNSET_TIME'];
	    	echo $separator;
			echo $row['EVENT_RAIN'];
	    	echo $separator;
	    	echo $row['RAIN_FREQUENCY'];
	    	echo $separator;
			}
			$result->free();
		}
		$conn->close();
?>
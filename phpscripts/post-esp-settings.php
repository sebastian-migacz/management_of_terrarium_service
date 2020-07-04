<?php

$servername = "localhost";
$dbname = "id13473022_espsmartterradb";
$username = "id13473022_smartadmin";
$password = "Smart_pass123";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
   
		$id = test_input($_POST["id"]);
		$sprinklerstatus = test_input($_POST["sprinklerstatus"]);
		$sprinklerhum = test_input($_POST["sprinklerhum"]);
		$waterfallstatus = test_input($_POST["waterfallstatus"]);
		$waterfallontime = test_input($_POST["waterfallontime"]);
		$waterfallofftime = test_input($_POST["waterfallofftime"]);
		$growlightstatus = test_input($_POST["growlightstatus"]);
		$growlightontime = test_input($_POST["growlightontime"]);
		$growlightofftime = test_input($_POST["growlightofftime"]);
		$rgbr = test_input($_POST["rgbr"]);
		$rgbg = test_input($_POST["rgbg"]);
		$rgbb = test_input($_POST["rgbb"]);
		$eventsunrise = test_input($_POST["eventsunrise"]);
		$sunrisetime = test_input($_POST["sunrisetime"]);
		$eventsunset = test_input($_POST["eventsunset"]);
		$sunsettime = test_input($_POST["sunsettime"]);
		$eventrain = test_input($_POST["eventrain"]);
		$rainfrequency = test_input($_POST["sunsettime"]);
		if($id==2){
			// Create connection
			$conn = new mysqli($servername, $username, $password, $dbname);
			// Check connection
			if ($conn->connect_error) {
				die("Connection failed: " . $conn->connect_error);
			} 
        
			$sql = "UPDATE SettingsData SET SPRINKLER_STATUS = $sprinklerstatus, SPRINKLER_HUM = $sprinklerhum, WATERFALL_STATUS = $waterfallstatus, WATERFALL_ON_TIME = $waterfallontime, WATERFALL_OFF_TIME = $waterfallofftime, GROWLIGHT_STATUS = $growlightstatus, GROWLIGHT_ON_TIME = $growlightontime, GROWLIGHT_OFF_TIME = $growlightofftime, RGB_R = $rgbr, RGB_G = $rgbg, RGB_B = $rgbb, EVENT_SUNRISE = $eventsunrise, SUNRISE_TIME = $sunrisetime, EVENT_SUNSET = $eventsunset, SUNSET_TIME = $sunsettime, EVENT_RAIN = $eventrain, RAIN_FREQUENCY = $rainfrequency WHERE ID = '2'";
        
			if ($conn->query($sql) === TRUE) {
				echo "Record updated successfully";
			} 
			else {
				echo "Error: " . $sql . "<br>" . $conn->error;
			}
    
			$conn->close();
		}
	
}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
<?php
include ('connect.php');
require __DIR__ . '/vendor/autoload.php';

use Kreait\Firebase\Factory;
use Kreait\Firebase\ServiceAccount;

class ESPData {
    private $error = "";
    private $firebase;
    private $database;

    public function __construct() {
        try {
            // Initialize Firebase
            $factory = (new Factory)
                ->withServiceAccount('path/to/your/firebase-credentials.json')
                ->withDatabaseUri('https://iot-simalas-default-rtdb.asia-southeast1.firebasedatabase.app');

            $this->firebase = $factory->createDatabase();
        } catch (Exception $e) {
            $this->error .= "Firebase connection error: " . $e->getMessage() . "<br>";
        }
    }

    public function evaluate($data) {
        foreach ($data as $key => $value) {
            if (empty($value)) {
                $this->error .= $key . " is empty!<br>";
            }
        }
        return $this->error;
    }

    // Function to get all data from Firebase
    public function getAllESPData() {
        try {
            // Assuming 'sensor' is your Firebase collection name
            $reference = $this->firebase->getReference('sensor');
            $snapshot = $reference->getSnapshot();
            
            if ($snapshot->exists()) {
                $data = [];
                foreach ($snapshot->getValue() as $key => $value) {
                    $data[] = [
                        'tegangan' => $value['tegangan'] ?? '',
                        'arus' => $value['arus'] ?? '',
                        'suhu' => $value['suhu'] ?? '',
                        'kelembapan' => $value['kelembapan'] ?? '',
                        'timestamp' => $value['timestamp'] ?? date('Y-m-d H:i:s')
                    ];
                }
                return $data;
            }
            return [];
        } catch (Exception $e) {
            $this->error .= "Error fetching Firebase data: " . $e->getMessage() . "<br>";
            return [];
        }
    }

    public function moveDataToNewTable() {
        // Get data from session
        if (isset($_SESSION["simalas_userid"], $_SESSION["simalas_nama"], $_SESSION["simalas_NIM"])) {
            $userid = $_SESSION["simalas_userid"];
            $username = $_SESSION["simalas_nama"];
            $userNIM = $_SESSION["simalas_NIM"];
        } else {
            $this->error .= "Session data is missing.<br>";
            return false;
        }
    
        // Get data from Firebase
        $sensorData = $this->getAllESPData();
    
        if (!empty($sensorData)) {
            $DB = new Database();
    
            foreach ($sensorData as $data) {
                $tegangan = $data['tegangan'];
                $arus = $data['arus'];
                $suhu = $data['suhu'];
                $kelembapan = $data['kelembapan'];
                $timestamp = $data['timestamp'];
    
                // Check if user data exists in data_sensor
                $checkQuery = "SELECT * FROM data_sensor WHERE userid = '$userid' AND NAMA = '$username' AND NIM = '$userNIM'";
                $result = $DB->read($checkQuery);
    
                if ($result && count($result) > 0) {
                    // Update if user exists
                    $updateQuery = "UPDATE data_sensor SET tegangan = '$tegangan', arus = '$arus', suhu = '$suhu', kelembapan = '$kelembapan', date = '$timestamp' 
                                    WHERE userid = '$userid' AND NAMA = '$username' AND NIM = '$userNIM'";
                    $DB->save($updateQuery);
                    error_log("Updated data for user: $username, NIM: $userNIM at " . date('Y-m-d H:i:s'));
                } else {
                    // Insert if user doesn't exist
                    $insertQuery = "INSERT INTO data_sensor (userid, NAMA, NIM, tegangan, arus, suhu, kelembapan, date) 
                                    VALUES ('$userid', '$username', '$userNIM', '$tegangan', '$arus', '$suhu', '$kelembapan', '$timestamp')";
                    $DB->save($insertQuery);
                    error_log("Inserted data for user: $username, NIM: $userNIM at " . date('Y-m-d H:i:s'));
                }
            }
            return true;
        } else {
            $this->error .= "No sensor data found in Firebase.<br>";
            return false;
        }
    }
    
    public function getSensorDataBySessionUser() {
        if (isset($_SESSION['simalas_userid'])) {
            $userid = $_SESSION['simalas_userid'];
    
            $query = "SELECT suhu, tegangan, arus, kelembapan FROM data_sensor WHERE userid = '$userid'";
            $DB = new Database();
            $result = $DB->read($query);
    
            if ($result && count($result) > 0) {
                return $result[0];
            }
        }
        return null;
    }

    public function getError() {
        return $this->error;
    }
}
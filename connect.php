
<?php

class Database
{
    private $host = "localhost"; 
    private $username = "root"; 
    private $password = "";
    private $db = "simalas"; 
    private $connection;

    function __construct()
    {
        $this->connection = mysqli_connect($this->host, $this->username, $this->password, $this->db);

        // Periksa koneksi
        if (mysqli_connect_errno()) {
            die("Koneksi gagal: " . mysqli_connect_error());
        }
    }

    function connect()
    {
        return $this->connection;
    }

    function read($query)
    {
        $result = mysqli_query($this->connection, $query);

        if (!$result) {
            return false;
        } else {
            $data = [];
            while ($row = mysqli_fetch_assoc($result)) {
                $data[] = $row;
            }
            return $data;
        }
    }

    function save($query)
    {
        $result = mysqli_query($this->connection, $query);

        if (!$result) {
            return false;
        } else {
            return true;
        }
    }

    // Metode untuk menyiapkan statement
    function prepare($query)
    {
        return mysqli_prepare($this->connection, $query);
    }

    // Metode untuk menutup koneksi
    function close()
    {
        mysqli_close($this->connection);
    }
}
?>

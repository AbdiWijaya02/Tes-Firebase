// Mengimpor modul Firebase
import { initializeApp } from "https://www.gstatic.com/firebasejs/11.0.0/firebase-app.js";
import { getDatabase, ref, onValue, set } from "https://www.gstatic.com/firebasejs/11.0.0/firebase-database.js";

// Konfigurasi Firebase
const firebaseConfig = {
    apiKey: "AIzaSyAuxypv3-xa77D-Q7PAdYrybAvolYT7haQ",
    authDomain: "iot-simalas.firebaseapp.com",
    databaseURL: "https://iot-simalas-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "iot-simalas",
    storageBucket: "iot-simalas.firebasestorage.app",
    messagingSenderId: "508299039650",
    appId: "1:508299039650:web:e1   4914d48cfbb4c0f958fb",
    measurementId: "G-V1N3XM2FQ6"
};

// Inisialisasi Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

// Fungsi untuk mengirim data ke backend PHP
function sendDataToBackend(data) {
    console.log("Data yang akan dikirim:", data);  // Cek data yang dikirim ke PHP

    fetch('/save_data.php', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
    })
    .then(response => {
        return response.text();  // Ubah menjadi text() untuk melihat semua respons
    })
    .then(responseData => {
        try {
            let jsonResponse = JSON.parse(responseData);  // Coba untuk parsing JSON
            console.log('Data berhasil disimpan di MySQL:', jsonResponse);
        } catch (e) {
            console.error('Error parsing JSON:', e);
            console.log('Response is not JSON:', responseData);  // Lihat apa yang sebenarnya diterima
        }
    })
    .catch(error => {
        console.error('Terjadi kesalahan:', error);
    });
}
// Fungsi untuk mengambil data dari Firebase
function getData() {
    const sensorRef = ref(database, 'sensor');
    onValue(sensorRef, (snapshot) => {
        const data = snapshot.val();
        console.log(data); // Log data untuk debugging

        if (data) {
            // Mengupdate elemen HTML
            document.getElementById('arus').innerText = data.arus + ' A';
            document.getElementById('daya').innerText = data.daya + ' W';
            document.getElementById('energi').innerText = data.energi + ' kWh';
            document.getElementById('kelembapan').innerText = data.kelembapan + ' %';
            document.getElementById('remaining').innerText = data.remaining + ' m';
            document.getElementById('suhu').innerText = data.suhu + ' °C';
            document.getElementById('tegangan').innerText = data.tegangan + ' V';
            document.getElementById('used').innerText = data.used + ' m';

            var currentTime = new Date().toLocaleTimeString();
            document.getElementById('last-update').innerText = "Last update: " + currentTime;

            // Mengirim data ke server PHP untuk disimpan ke MySQL
            sendDataToBackend(data);
        }
    });
}


// Fungsi untuk toggle SSR (dengan tombol disable sementara)
export function toggleSSR() {
    const ssrRef = ref(database, 'control/ssrState');
    const toggleButton = document.getElementById('toggleButton');

    onValue(ssrRef, (snapshot) => {
        const prevState = snapshot.val();
        const newState = !prevState;  // Toggle SSR state

        set(ssrRef, newState)
            .then(() => {
                document.getElementById('ssrStatus').innerText = `SSR Status: ${newState ? 'ON' : 'OFF'}`;
            })
            .catch((error) => {
                console.error("Error updating SSR state:", error);
            });
    });
}

// Memanggil fungsi getData ketika halaman dimuat
window.onload = function() {
    getData();

    // Menambahkan event listener untuk tombol toggleSSR
    document.getElementById('toggleButton').addEventListener('click', toggleSSR);
};

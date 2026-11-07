// Game Ular Sederhana (Console) - C++
// Kontrol: W/A/S/D atau tombol panah | Q = keluar
//
// Compile:
//   Windows (MinGW) / Linux / macOS:  g++ snake.cpp -o snake -std=c++11
// Jalankan:
//   Windows: snake.exe      Linux/macOS: ./snake

#include <iostream>
#include <deque>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#endif

using namespace std;

const int LEBAR = 40;
const int TINGGI = 20;

struct Titik {
    int x, y;
    bool operator==(const Titik& o) const { return x == o.x && y == o.y; }
};

enum Arah { ATAS, BAWAH, KIRI, KANAN };

// ---------------- Input & terminal (beda per OS) ----------------
#ifdef _WIN32

void siapkanTerminal() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | 0x0004); // aktifkan ANSI escape code
}
void pulihkanTerminal() {}

// Mengembalikan 'w','a','s','d','q' atau 0 jika tidak ada input
char bacaTombol() {
    if (!_kbhit()) return 0;
    int c = _getch();
    if (c == 0 || c == 224) { // tombol panah
        switch (_getch()) {
            case 72: return 'w';
            case 80: return 's';
            case 75: return 'a';
            case 77: return 'd';
        }
        return 0;
    }
    return (char)tolower(c);
}

#else

termios terminalAsli;

void pulihkanTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &terminalAsli);
}
void siapkanTerminal() {
    tcgetattr(STDIN_FILENO, &terminalAsli);
    termios baru = terminalAsli;
    baru.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &baru);
    atexit(pulihkanTerminal);
}

bool adaInput() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    timeval tv = {0, 0};
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}

char bacaTombol() {
    if (!adaInput()) return 0;
    char c;
    if (read(STDIN_FILENO, &c, 1) <= 0) return 0;
    if (c == 27 && adaInput()) { // escape sequence tombol panah
        char s[2];
        if (read(STDIN_FILENO, &s[0], 1) > 0 && s[0] == '[' &&
            read(STDIN_FILENO, &s[1], 1) > 0) {
            switch (s[1]) {
                case 'A': return 'w';
                case 'B': return 's';
                case 'D': return 'a';
                case 'C': return 'd';
            }
        }
        return 0;
    }
    return (char)tolower(c);
}

#endif

// ---------------- Logika game ----------------
Titik buatMakanan(const deque<Titik>& ular) {
    while (true) {
        Titik m = {rand() % LEBAR, rand() % TINGGI};
        bool bentrok = false;
        for (const Titik& t : ular)
            if (t == m) { bentrok = true; break; }
        if (!bentrok) return m;
    }
}

void gambar(const deque<Titik>& ular, const Titik& makanan, int skor) {
    string layar = "\033[H"; // kursor ke pojok kiri atas
    layar += "=== GAME ULAR ===   Skor: " + to_string(skor) + "   (Q = keluar)\n";
    layar += "+" + string(LEBAR, '-') + "+\n";
    for (int y = 0; y < TINGGI; y++) {
        layar += "|";
        for (int x = 0; x < LEBAR; x++) {
            Titik p = {x, y};
            if (p == ular.front()) layar += 'O';
            else if (p == makanan) layar += '*';
            else {
                bool badan = false;
                for (const Titik& t : ular)
                    if (t == p) { badan = true; break; }
                layar += badan ? 'o' : ' ';
            }
        }
        layar += "|\n";
    }
    layar += "+" + string(LEBAR, '-') + "+\n";
    cout << layar << flush;
}

// Mengembalikan true jika pemain ingin main lagi
bool mainSatuRonde(int& skorAkhir) {
    deque<Titik> ular;
    ular.push_back({LEBAR / 2, TINGGI / 2});
    ular.push_back({LEBAR / 2 - 1, TINGGI / 2});
    ular.push_back({LEBAR / 2 - 2, TINGGI / 2});

    Arah arah = KANAN;
    Titik makanan = buatMakanan(ular);
    int skor = 0;

    cout << "\033[2J"; // bersihkan layar

    while (true) {
        // Baca semua input yang menumpuk, ambil arah terakhir yang valid
        char k;
        while ((k = bacaTombol()) != 0) {
            if (k == 'q') { skorAkhir = skor; return false; }
            if (k == 'w' && arah != BAWAH) arah = ATAS;
            else if (k == 's' && arah != ATAS) arah = BAWAH;
            else if (k == 'a' && arah != KANAN) arah = KIRI;
            else if (k == 'd' && arah != KIRI) arah = KANAN;
        }

        // Gerakkan kepala
        Titik kepala = ular.front();
        if (arah == ATAS) kepala.y--;
        else if (arah == BAWAH) kepala.y++;
        else if (arah == KIRI) kepala.x--;
        else kepala.x++;

        // Tabrak dinding?
        if (kepala.x < 0 || kepala.x >= LEBAR || kepala.y < 0 || kepala.y >= TINGGI)
            break;

        // Tabrak badan sendiri?
        bool tabrak = false;
        for (const Titik& t : ular)
            if (t == kepala) { tabrak = true; break; }
        if (tabrak) break;

        ular.push_front(kepala);
        if (kepala == makanan) {
            skor += 10;
            makanan = buatMakanan(ular);
        } else {
            ular.pop_back();
        }

        gambar(ular, makanan, skor);

        // Makin tinggi skor, makin cepat (minimal 50 ms)
        int jeda = 130 - skor / 2;
        if (jeda < 50) jeda = 50;
        this_thread::sleep_for(chrono::milliseconds(jeda));
    }

    skorAkhir = skor;
    cout << "\n  GAME OVER! Skor akhir: " << skor << "\n";
    cout << "  Tekan R untuk main lagi, atau Q untuk keluar.\n";

    while (true) {
        char k = bacaTombol();
        if (k == 'r') return true;
        if (k == 'q') return false;
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

int main() {
    srand((unsigned)time(nullptr));
    siapkanTerminal();
    cout << "\033[?25l"; // sembunyikan kursor

    int skor = 0;
    while (mainSatuRonde(skor)) {}

    cout << "\033[?25h\033[2J\033[H"; // tampilkan kursor lagi
    cout << "Terima kasih sudah bermain! Skor terakhir: " << skor << "\n";
    pulihkanTerminal();
    return 0;
}

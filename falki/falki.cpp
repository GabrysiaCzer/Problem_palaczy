#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

// Definicje mutexów do synchronizacji dostępu do zasobów i ekranu
std::mutex mtxUbijacze, mtxZapalki, mtxEkran;
// Zmienne warunkowe do kontrolowania dostępu do ubijaczy i zapałek
std::condition_variable cvUbijacze, cvZapalki;
// Liczniki dostępnych ubijaczy i zapałek
int dostepneUbijacze, dostepneZapalki;
// Kolejki FIFO przechowujące ID palaczy oczekujących na ubijacze i zapałki
std::queue<int> kolejkaUbijacze, kolejkaZapalki;

// Funkcja do wyświetlania komunikatów z synchronizacją dostępu do ekranu
void wyswietlKomunikat(const std::string& komunikat) {
    std::lock_guard<std::mutex> lock(mtxEkran); // Blokada mutexu ekranu
    std::cout << komunikat << std::endl;
}

// Funkcja reprezentująca działanie palacza
void palacz(int id) {
    while (true) {
        // Żądanie ubijacza
        {
            std::unique_lock<std::mutex> lock(mtxUbijacze); // Blokada mutexu ubijaczy
            kolejkaUbijacze.push(id); // Dodanie ID do kolejki
            // Oczekiwanie na dostępność ubijacza i bycie pierwszym w kolejce
            cvUbijacze.wait(lock, [id] { return dostepneUbijacze > 0 && kolejkaUbijacze.front() == id; });
            dostepneUbijacze--; // Zajęcie ubijacza
            kolejkaUbijacze.pop(); // Usunięcie ID z kolejki
            wyswietlKomunikat("Palacz " + std::to_string(id) + " ubija fajke.");
        }

        // Symulacja ubijania fajki
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Zwrot ubijacza
        {
            std::lock_guard<std::mutex> lock(mtxUbijacze); // Blokada mutexu ubijaczy
            dostepneUbijacze++; // Zwolnienie ubijacza
            cvUbijacze.notify_one(); // Powiadomienie jednego oczekującego palacza
            wyswietlKomunikat("Palacz " + std::to_string(id) + " odlozyl ubijacz.");
        }

        // Żądanie pudełka zapałek
        {
            std::unique_lock<std::mutex> lock(mtxZapalki); // Blokada mutexu zapałek
            kolejkaZapalki.push(id); // Dodanie ID do kolejki
            // Oczekiwanie na dostępność zapałek i bycie pierwszym w kolejce
            cvZapalki.wait(lock, [id] { return dostepneZapalki > 0 && kolejkaZapalki.front() == id; });
            dostepneZapalki--; // Zajęcie zapałek
            kolejkaZapalki.pop(); // Usunięcie ID z kolejki
            wyswietlKomunikat("Palacz " + std::to_string(id) + " zapala fajke.");
        }

        // Symulacja zapalania fajki
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Zwrot pudełka zapałek
        {
            std::lock_guard<std::mutex> lock(mtxZapalki); // Blokada mutexu zapałek
            dostepneZapalki++; // Zwolnienie zapałek
            cvZapalki.notify_one(); // Powiadomienie jednego oczekującego palacza
            wyswietlKomunikat("Palacz " + std::to_string(id) + " odlozyl pudelko z zapalkami.");
        }

        wyswietlKomunikat("Palacz " + std::to_string(id) + " pali fajke.");
        // Symulacja palenia fajki
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main() {
    const int k = 5; // Liczba palaczy
    const int l = 2; // Liczba ubijaczy
    const int m = 3; // Liczba pudełek zapałek
    dostepneUbijacze = l;
    dostepneZapalki = m;

    std::vector<std::thread> threads;

    // Tworzenie i uruchamianie wątków palaczy
    for (int i = 0; i < k; ++i) {
        threads.push_back(std::thread(palacz, i));
    }

    // Oczekiwanie na zakończenie pracy wątków
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}

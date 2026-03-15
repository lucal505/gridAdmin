# gridAdmin

Server + controller pentru managementul de la distanta al unui grid de statii Linux via SSH.

Serverul (`gridServer`) ruleaza pe o masina cu acces la retea si se ocupa de conexiunile SSH catre statii. Controllerul (`gridController`) se conecteaza la server si permite trimiterea de comenzi catre statii selectate.

## Functionalitati

- Verificare stare statii (online/offline)
- Wake on LAN
- Shutdown remote
- Executie comenzi bash custom pe una sau mai multe statii simultan

## Getting Started

### 1. Clonare
```bash
git clone https://github.com/lucal505/gridAdmin.git
cd gridAdmin
```

### 2. Instalare dependinte de sistem
```bash
sudo apt install gcc libsqlite3-dev sshpass
```

### 3. Build
```bash
make
```

### 4. Configurare (vezi sectiunea de mai jos)

### 5. Rulare
```bash
./gridServer       # pe masina server
./gridController   # pe masina client
```

## Configurare

### 1. Credentiale SSH
In `include/data.h` sunt definite credentialele folosite pentru conexiunile SSH catre statii:

```c
#define USER "test"
#define PASS "testpass"
```

Inlocuieste `test` si `testpass` cu userul si parola comune ale statiilor din retea.

### 2. Baza de date (grid.db)
`db/grid.db` este o baza de date SQLite care contine lista statiilor din grid. Trebuie populata cu datele reale ale statiilor tale inainte de a rula serverul.

Schema tabelului:
```sql
CREATE TABLE hosts (
    id      INTEGER PRIMARY KEY,
    ip      TEXT NOT NULL,
    mac     TEXT NOT NULL,
    name    TEXT
);
```

Exemplu de populare:
```sql
INSERT INTO hosts (id, ip, mac, name) VALUES (1, '192.168.1.101', 'AA:BB:CC:DD:EE:FF', 'station-01');
INSERT INTO hosts (id, ip, mac, name) VALUES (2, '192.168.1.102', '11:22:33:44:55:66', 'station-02');
```

Poti edita baza de date cu:
```bash
sqlite3 db/grid.db
```

### 3. IP-ul serverului
In `src/gridController.c` este definit IP-ul la care controllerul se conecteaza:

```c
#define SERVER_IP "127.0.0.1"
```

Schimba-l cu IP-ul masinii pe care ruleaza `gridServer` daca nu rulezi totul local.

## Build

```bash
make
```

Sau separat:
```bash
make gridServer
make gridController
```

Pentru a sterge binarele:
```bash
make clean
```

## Rulare

Pe masina server (cu acces la retea):
```bash
./gridServer
```

Pe masina client (de pe orice masina din retea):
```bash
./gridController
```

## Structura proiectului

```
gridAdmin/
├── src/
│   ├── gridServer.c        # server principal
│   └── gridController.c    # client CLI
├── include/
│   ├── comms.h             # trimitere/receptie mesaje TCP
│   ├── data.h              # structuri de date si constante
│   ├── handlers.h          # handlere pentru clienti si conexiuni SSH
│   ├── manage_connections.h# initializare/inchidere conexiuni SSH
│   └── misc.h              # utilitare (WoL, validare ID)
├── db/
│   └── grid.db             # baza de date SQLite cu statiile din grid
├── Makefile
└── README.md
```

# gridAdmin

Server + controller for remote management of a Linux station grid via SSH.

The server (`gridServer`) runs on a machine with network access and handles SSH connections to the stations. The controller (`gridController`) connects to the server and allows sending commands to selected stations.

## Features

- Check station status (online/offline)
- Wake on LAN
- Remote shutdown
- Execute custom bash commands on one or more stations simultaneously

## Getting Started

### 1. Clone
```bash
git clone https://github.com/lucal505/gridAdmin.git
cd gridAdmin
```

### 2. Install system dependencies
```bash
sudo apt install gcc libsqlite3-dev sshpass
```

### 3. Build
```bash
make
```

### 4. Configure (see section below)

### 5. Run
```bash
./gridServer       # on the server machine
./gridController   # on the client machine
```

## Configuration

### 1. SSH Credentials
The credentials used for SSH connections to the stations are defined in `include/data.h`:

```c
#define USER "test"
#define PASS "testpass"
```

Replace `test` and `testpass` with the common username and password of the stations in your network.

### 2. Database (grid.db)
`db/grid.db` is a SQLite database containing the list of stations in the grid. It must be populated with your actual station data before running the server.

Table schema:
```sql
CREATE TABLE hosts (
    id      INTEGER PRIMARY KEY,
    ip      TEXT NOT NULL,
    mac     TEXT NOT NULL,
    name    TEXT
);
```

Example entries:
```sql
INSERT INTO hosts (id, ip, mac, name) VALUES (1, '192.168.1.101', 'AA:BB:CC:DD:EE:FF', 'station-01');
INSERT INTO hosts (id, ip, mac, name) VALUES (2, '192.168.1.102', '11:22:33:44:55:66', 'station-02');
```

You can edit the database with:
```bash
sqlite3 db/grid.db
```

### 3. Server IP
The IP address the controller connects to is defined in `src/gridController.c`:

```c
#define SERVER_IP "127.0.0.1"
```

Change it to the IP of the machine running `gridServer` if you are not running everything locally.

## Build

```bash
make
```

Or individually:
```bash
make gridServer
make gridController
```

To remove compiled binaries:
```bash
make clean
```

## Running

On the server machine (with network access):
```bash
./gridServer
```

On the client machine (from anywhere in the network):
```bash
./gridController
```

## Project Structure

```
gridAdmin/
├── src/
│   ├── gridServer.c        # main server
│   └── gridController.c    # CLI client
├── include/
│   ├── comms.h             # TCP message send/receive
│   ├── data.h              # data structures and constants
│   ├── handlers.h          # client and SSH connection handlers
│   ├── manage_connections.h# SSH connection init/teardown
│   └── misc.h              # utilities (WoL, ID validation)
├── db/
│   └── grid.db             # SQLite database with grid stations
├── Makefile
└── README.md
```

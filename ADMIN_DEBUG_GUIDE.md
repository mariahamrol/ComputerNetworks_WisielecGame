# Admin Client Debugging Guide

## Quick Start

### 1. Login as Admin
```
Nick: admin
Hasło admina: 3edcvfr4
```

### 2. Admin Commands Menu
Once logged in as admin, you'll see:
```
[ADMIN] a - lista gier | u - użytkownicy | i - info o grze | x - zakończ grę | q - wyjdź:
```

## Command Reference

### **a** - Lista wszystkich gier
Shows all games on the server (both active games and waiting rooms).

**Output format:**
```
=== Wszystkie gry (admin) ===
Gra 1 | Graczy: 3 | Status: AKTYWNA | Owner: player1
Gra 2 | Graczy: 2 | Status: POKÓJ | Owner: player2
Razem gier: 2, razem graczy: 5
```

**Status meanings:**
- `AKTYWNA` - Game is currently being played
- `POKÓJ` - Game room exists but hasn't started yet

---

### **u** - Lista wszystkich użytkowników
Shows every client connected to the server.

**Output format:**
```
=== Wszyscy użytkownicy (admin) ===
Nick: player1 | Stan: IN_GAME | Gra: 1 | Życia: 5 | Punkty: 30
Nick: player2 | Stan: IN_ROOM | Gra: 2 | Życia: 0 | Punkty: 0
Nick: spectator | Stan: LOBBY | Gra: brak | Życia: 0 | Punkty: 0
Razem użytkowników: 3
```

**States:**
- `CONNECTED` - Just connected, not logged in yet
- `LOGGING_IN` - In process of logging in
- `WAIT_ADMIN_PWD` - Waiting for admin password
- `LOBBY` - In lobby, not in any game
- `IN_ROOM` - In game room waiting for start
- `IN_GAME` - Actively playing
- `ADMIN` - Logged in as admin

---

### **i** - Szczegóły gry (Info)
Get comprehensive details about a specific game.

**Usage:**
```
i
ID gry do sprawdzenia: 1
```

**Output format:**
```
=== Szczegóły gry ID: 1 (admin) ===
Status: AKTYWNA
Owner: player1
Graczy: 3

*** SŁOWO (admin): HANGMAN ***
Stan odgadywania: H A N G _ A N
Odgadnięte litery (wszyscy): H A N G

--- Gracze ---
1. player1 | Życia: 5 | Punkty: 40 | Aktywny: TAK | Owner: TAK
   Odgadnięte przez tego gracza: H A
2. player2 | Życia: 3 | Punkty: 30 | Aktywny: TAK | Owner: NIE
   Odgadnięte przez tego gracza: N G
3. player3 | Życia: 0 | Punkty: 10 | Aktywny: NIE | Owner: NIE
   Odgadnięte przez tego gracza: Z X
==================
```

**Key information:**
- **SŁOWO (admin)** - The actual word (only admin sees this!)
- **Stan odgadywania** - What players currently see (with blanks)
- **Odgadnięte litery (wszyscy)** - All letters that have been guessed
- **Per player:**
  - Lives remaining
  - Points scored
  - Active status (eliminated players show NIE)
  - Owner status
  - Individual letters they guessed

---

### **x** - Zakończ grę (Terminate)
Immediately end any active game and return all players to lobby.

**Usage:**
```
x
ID gry do zakończenia: 1
```

**Result:**
- Game is deleted from server
- All players receive `MSG_GAME_END` 
- Players are returned to lobby
- Confirmation message: `Gra zakończona przez admina`

---

## Debugging Workflow Example

### Scenario: Monitor a game in progress

1. **Check active games:**
   ```
   Command: a
   Output: Gra 1 | Graczy: 4 | Status: AKTYWNA | Owner: alice
   ```

2. **Check all connected users:**
   ```
   Command: u
   Output: Shows 4 players, all in game 1
   ```

3. **Get detailed game info:**
   ```
   Command: i
   Enter: 1
   Output: Shows actual word "HANGMAN", current progress "H_N__AN", 
           and individual stats for all 4 players
   ```

4. **Monitor player progress:**
   - See who's guessing what letters
   - Check lives and points
   - Identify eliminated players (Aktywny: NIE)

5. **End game if needed:**
   ```
   Command: x
   Enter: 1
   Output: Gra zakończona przez admina
   ```

---

## Testing Checklist

### Basic Functionality
- [ ] Login as admin with correct password
- [ ] Wrong password shows error
- [ ] List games shows all games (active + rooms)
- [ ] List users shows all connected clients
- [ ] Game details shows actual word
- [ ] Game details shows all player stats
- [ ] Terminate game works and notifies players

### Edge Cases
- [ ] List empty games (no games exist)
- [ ] List empty users (only admin connected)
- [ ] Get details of non-existent game ID
- [ ] Terminate already-ended game
- [ ] View game details for waiting room (not started yet)

### Multi-Client Testing
1. Start server
2. Connect as admin in terminal 1
3. Connect as regular players in terminals 2-5
4. Create games, join rooms
5. Use admin to monitor everything
6. Start a game
7. Use admin to see the word and track progress
8. Use admin to terminate game mid-play

---

## Common Issues

### "Cannot connect to server"
- Ensure server is running on port 12345
- Check firewall settings

### "Błędne hasło admina"
- Password is case-sensitive: `3edcvfr4`
- Nick must be exactly: `admin`

### Empty lists
- Normal if no games/users exist yet
- Connect other clients to populate data

### Game details show garbage
- Ensure game ID exists (check with `a` first)
- Active games show more info than rooms

---

## Pro Tips

1. **Quick workflow:** `u` → `a` → `i <game_id>` gives complete overview
2. **Monitor word progress:** Use `i` repeatedly during active games
3. **Track eliminations:** Watch "Aktywny: NIE" in game details
4. **Debug disconnects:** Use `u` to see if clients are still connected
5. **Clean up:** Use `x` to remove stale/broken games

---

## Network Protocol Summary

The admin client now sends:
- `MSG_ADMIN_LIST_GAMES_REQ` (no payload)
- `MSG_ADMIN_LIST_USERS_REQ` (no payload)  
- `MSG_ADMIN_GAME_DETAILS_REQ` (with game_id)
- `MSG_ADMIN_TERMINATE_GAME` (with game_id)

And receives:
- `MsgAdminGamesList` - all games with status
- `MsgAdminUsersList` - all connected users
- `MsgAdminGameDetails` - complete game state including answer
- `MSG_ADMIN_TERMINATE_OK/FAIL` - termination confirmation

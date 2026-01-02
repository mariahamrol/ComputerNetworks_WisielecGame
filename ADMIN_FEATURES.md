# Admin Features Documentation

## Overview
The admin interface now has comprehensive visibility into all server activity including users, games, and detailed player statistics.

## Admin Commands

### 1. **List All Games** (`MSG_ADMIN_LIST_GAMES_REQ`)
Shows all games on the server (both active and inactive).

**Request:** `MSG_ADMIN_LIST_GAMES_REQ` (no payload)

**Response:** `MSG_ADMIN_GAMES_LIST` with `MsgAdminGamesList`
```cpp
typedef struct {
    uint32_t game_id;
    uint8_t players_count;
    uint8_t is_active;        // 1 = active game, 0 = room/lobby
    char owner[MAX_NICK_LEN];
} AdminGameInfo;

typedef struct {
    uint32_t games_count;
    AdminGameInfo games[MAX_GAMES];
} MsgAdminGamesList;
```

**What you get:**
- Game ID
- Number of players
- Active status (1 = game in progress, 0 = waiting room)
- Owner nickname

---

### 2. **List All Users** (`MSG_ADMIN_LIST_USERS_REQ`)
Shows every client connected to the server.

**Request:** `MSG_ADMIN_LIST_USERS_REQ` (no payload)

**Response:** `MSG_ADMIN_USERS_LIST` with `MsgAdminUsersList`
```cpp
typedef struct {
    char nick[MAX_NICK_LEN];
    uint8_t state;           // ClientState enum value
    int32_t game_id;         // -1 if not in game
    uint8_t lives;
    uint16_t points;
} AdminUserInfo;

typedef struct {
    uint32_t users_count;
    AdminUserInfo users[64];
} MsgAdminUsersList;
```

**What you get:**
- Nickname
- Connection state (see states below)
- Game ID they're in (-1 if none)
- Current lives
- Current points

**Client States:**
- `0` = STATE_CONNECTED (just connected)
- `1` = STATE_LOGGING_IN
- `2` = STATE_WAIT_ADMIN_PASSWORD
- `3` = STATE_LOBBY
- `4` = STATE_IN_ROOM (waiting for game to start)
- `5` = STATE_IN_GAME (actively playing)
- `6` = STATE_ADMIN

---

### 3. **Get Game Details** (`MSG_ADMIN_GAME_DETAILS_REQ`)
Get comprehensive information about a specific game including all players, their stats, and the current word.

**Request:** `MSG_ADMIN_GAME_DETAILS_REQ` with `MsgGameIdReq`
```cpp
typedef struct {
    uint32_t game_id;
} MsgGameIdReq;
```

**Response:** `MSG_ADMIN_GAME_DETAILS` with `MsgAdminGameDetails`
```cpp
typedef struct {
    char nick[MAX_NICK_LEN];
    uint8_t lives;
    uint16_t points;
    uint8_t is_active;       // 1 = still playing, 0 = eliminated
    uint8_t is_owner;        // 1 = room owner
    char guessed_letters[ALPHABET_SIZE];
} AdminPlayerInfo;

typedef struct {
    uint32_t game_id;
    uint8_t is_active;
    uint8_t player_count;
    char owner[MAX_NICK_LEN];
    char word[MAX_WORD_LEN];              // The actual word (visible to admin)
    char word_guessed[MAX_WORD_LEN];      // Current guessed state (e.g., "H_NG_AN")
    char guessed_letters[ALPHABET_SIZE];  // All letters guessed by anyone
    uint8_t word_length;
    AdminPlayerInfo players[MAX_PLAYERS];
} MsgAdminGameDetails;
```

**What you get:**
- Game ID and status
- Number of players
- Game owner
- **The actual word** (only admin sees this!)
- Current guessed state
- All guessed letters
- For each player:
  - Nickname
  - Lives remaining
  - Points scored
  - Active status
  - Owner status
  - Their individual guessed letters

---

### 4. **Terminate Game** (`MSG_ADMIN_TERMINATE_GAME`)
End any active game immediately.

**Request:** `MSG_ADMIN_TERMINATE_GAME` with `MsgGameIdReq`

**Response:** 
- `MSG_ADMIN_TERMINATE_OK` on success
- `MSG_ADMIN_TERMINATE_FAIL` if game doesn't exist or isn't active

After termination, all players are returned to the lobby.

---

## Usage Example Flow

1. **Admin logs in** → receives `MSG_ADMIN_LOGIN_OK`

2. **List all users** to see who's online:
   ```
   Send: MSG_ADMIN_LIST_USERS_REQ
   Receive: MsgAdminUsersList with all connected clients
   ```

3. **List all games** to see what's happening:
   ```
   Send: MSG_ADMIN_LIST_GAMES_REQ
   Receive: MsgAdminGamesList with all games (active and waiting)
   ```

4. **Get detailed info** on a specific game:
   ```
   Send: MSG_ADMIN_GAME_DETAILS_REQ with game_id = 5
   Receive: MsgAdminGameDetails showing the word, all players, their lives, points, etc.
   ```

5. **Terminate a game** if needed:
   ```
   Send: MSG_ADMIN_TERMINATE_GAME with game_id = 5
   Receive: MSG_ADMIN_TERMINATE_OK
   Players automatically receive: MSG_GAME_END
   ```

---

## Implementation Notes

### Server-Side Changes
- `handle_admin_list_games()` - Now shows ALL games (not just active)
- `handle_admin_list_users()` - New function to list all connected clients
- `handle_admin_game_details()` - New function to get detailed game info
- All handlers verify `client->state == STATE_ADMIN` before processing

### Message Processing
Add these cases to your client's message handler:
```cpp
case MSG_ADMIN_GAMES_LIST:
    // Process MsgAdminGamesList
    break;
case MSG_ADMIN_USERS_LIST:
    // Process MsgAdminUsersList
    break;
case MSG_ADMIN_GAME_DETAILS:
    // Process MsgAdminGameDetails
    break;
```

### Security
- All admin functions check `client->state == STATE_ADMIN`
- Only authenticated admins can access these endpoints
- Admin password: `"3edcvfr4"` (defined in server.cpp)

---

## Benefits of This Approach

✅ **Complete visibility** - Admin sees everything happening on the server
✅ **Detailed insights** - Individual player stats, guessed letters, actual words
✅ **Both active and waiting games** - Monitor games before they start
✅ **User tracking** - See all connected users and their current state
✅ **Scalable** - Supports up to 64 users and 32 games
✅ **Separation of concerns** - Three distinct commands for different use cases

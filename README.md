# WordExchange Game

Welcome to the WordExchange game! This is a multiplayer CLI game where players connect to a server to play word-matching games. The game features a login/registration system, matchmaking, and leaderboards.

## Features

- **Login and Registration:** Secure user authentication with hashed passwords.
- **Matchmaking:** Players are paired based on their availability and can play with others in real-time.
- **Game Sessions:** Keep track of ongoing sessions, rounds, and gameplay results.
- **Leaderboards:** View global and personal rankings based on game performance.
- **Timeouts and Disconnect Handling:** Ensures smooth gameplay even if players disconnect or fail to respond in time.

## Prerequisites

- **C++ Compiler:** Ensure you have a C++ compiler installed.
- **SQLite:** Required for database operations.
- **CMake:** Used for building the project.
- **CLion (optional):** Recommended IDE for development.
## Installation

**Clone the Repository:**

   ```bash
   git clone https://github.com/TamarZerekidze/WordExchange.git
```
**Clone Client repository.**

**Create copy(ies) of Client repository.**

**Run Server class first, then Client class(es).**

**Follow the on-screen prompts to log in or register, and then join a game.**

## Code Structure

- **`User` Class:** Manages user data, including username and hashed password.
- **`UserDAO` Class:** Handles database operations related to users.
- **`GameDAO` Class:** Manages game sessions and gameplay data.
- **`Server` Class:** The main server class handling client connections, matchmaking, and game sessions.
- **`Client` Class:** The client-side application for interacting with the server.
- **`Session` Class:** Manages session details including player information, round numbers, and session times.
- **`Service` Class:** Provides various game-related services, such as handling game logic and interactions between the server and clients.



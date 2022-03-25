# Messages {#messages}

Each log messages has its own unique code, it starts with a letter which indicates the level:

| Letter | Level                            |
| ------ | -------------------------------- |
| **F**  | [Fatal](messages/fatal.md)       |
| **C**  | [Critical](messages/critical.md) |
| **E**  | [Error](messages/error.md)       |
| **W**  | [Warning](messages/warning.md)   |
| **N**  | [Notice](messages/notice.md)     |
| **I**  | [Info](messages/info.md)         |
| **D**  | [Debug](messages/debug.md)       |

And a four digit number which indicates the message, the numeric range is divided into different categories for quickly indentifing the source of the message, see below:

| Numbers       | Category                    |
| ------------- | --------------------------- |
| 1000 … 1999 | Traintastic core components |
| 2000 … 2999 | Hardware interfacing        |
| 3000 … 3999 | *unused*                    |
| 4000 … 4999 | *unused*                    |
| 5000 … 5999 | *unused*                    |
| 6000 … 6999 | *unused*                    |
| 7000 … 7999 | *unused*                    |
| 8000 … 8999 | *unused*                    |
| 9000 … 9999 | Lua scripting               |

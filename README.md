# austerity
A card game with a token economy

use `make` to compile

use `make BUILD=test` to compile with debugging information

use `make BUILD=verbose` to compile with LOTS of debugging information

### players

run a player with `./type players id`

`type` is one of `shenzi`, `banzai`, or `ed`,

`players` is a number between 2 and 26 representing the maximum number of players in the game,

`id` is a number between 0 and 25 representing the id of this player

### hub

run the hub with `./austerity tokens points deck player player [player ...]`

`tokens` is starting a positive integer representing the number of tokens,

`points` is starting a positive integer representing the number of points required to win,

`deck` is file containing the cards,

`player` is the player program to execute

# Order Book:
`
An order book is a core market data structure that stores active buy and sell orders for an asset. It enables matching engines to decide when trades occur.
`
## Sides
`
   The book has two sides:
`

### Bid Side (Buy Orders):
`Buyers specify the maximum price they are willing to pay.`

### Ask Side (Sell Orders):
`Sellers specify the minimum price they are willing to accept. `

`These things are stored in form of Price sensitive order as theta decay plays an important part `

### Theta decay:
` The concept which define the loss of premium value over time on the Extrinsic value `

### Crossed Prices:
` If a new order's price crosses over the best bit(a buy option) or ask( a sell option) price then a trade is executed `
#### Scenerio:
##### Case 1: Incoming Buy Order
A buy order can match existing sell orders if:
buy_price ≥ best_ask
Because the buyer is willing to pay enough to meet or exceed what sellers are asking.
`
`
##### Case 2: Incoming Sell Order
A sell order can match existing buy orders if:
sell_price ≤ best_bid
Because the seller is willing to accept the same or lower price than buyers are bidding.
`

#### Extras:
`
The makefile doesnt follow a order of defintion first and then mention 
Make wants:
Compile each .cpp → .o
Link all .o files together → final executable
`

### So Here's the Flow:
`
   1. A file would be needed to store the basic structure of a order it can be buy or sell, comes the order.h 
   2. Then a book consisting of these order are made so that it could store the list of all orders, comes the orderbook.h which takes the order structure from its previus file
   3. Now we the functions to actually do something so comes the Orderbook.cpp 
   4. To orchestrate along with the entry point comes the main.cpp 
   5. After this we want to keep sure that the executabe file is made in the controlled environment we want so comes the Makefile which is a automation tool 

`
### Things learned:
`
.h = declarations (what exists)
.cpp = definitions (how it works)
main.cpp includes .h to know the functions
After this files are compiled to .o files
The linker later matches declarations to definitions
`

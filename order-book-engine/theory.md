# Order Book:
`
Its the core data structure that store the order query form which transactions are made into the market along with storing them before having the chance to implement the order in case of not finding a condition mentioned by the buyer
`
## Sides
`
   It has 2 sides which are:
`

### Buyer side:
`Which shows the buy orders where people are agreeing to pay for a premium with`

### Seller side:
`Which shows the sell order where people are agreeing to pay a premium in `

`These things are stored in form of Price sensitive order as theta decay plays an important part `

### Theta decay:
` The concept which define the loss of premium value over time on the Extrinsic value `

### Crossed Prices:
` 
If a new order's price is crossing over the best bit(a buy option) or ask( a sell option) price then a trade is executed 
`
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
//Condition?TState:FState
//
//Conditions
//----------
//A<action>      - an action - not really a condition
//O<object><loc> - object at location
//F<#>           - food >= # * 10 percent
//R<percent>     - random
//
//
//<object>                                       <mappos>
//----------                                     --------
//1   - anything                                  front
//0   - nothing                                     ^
//F   - any edible                                  |
//P   - any preditor                                |
//L   - any flora                                a  b  c
//A   - any fauna
//~c  - flora/fauna obj with tag 'c'          d  e  f  g  h
//
//                                            i  j  k  l  m
//<location>     <dist>        <side>
//-------------  ------        ------         n  o  p  q  r
//<mappos>       C   - close   R - right            -
//<dist><side>   M   - medium  L - left          s |X| t
//               F   - far     F - front            -
//               A   - any     B - back          u  v  w
//                             A - any
//
//<action>                       
//--------                       
//E      - eat             
//T<dir> - turn            
//F      - move forward    
//B      - move back
//S      - sleep
//
//
//A [E|F|B|S|T#]
//O (1|0|F|P|L|A|~c0)([a-w]|([CMFA][RLFBA]))
//F #
//R ##
//
//
//pState
//------
//pszCond - Condition
//iTNode  -
//iFNode  -


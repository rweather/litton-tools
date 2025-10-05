Usage of the tracks on the drum
===============================

<table border="1">
<tr><td><b>Track</b></td><td><b>Address Range</b></td><td><b>Description</b></td></tr>
<tr><td>0</td><td>$000 to $07F</td><td>Scratchpad and general storage</td></tr>
<tr><td>1</td><td>$080 to $0FF</td><td> </td></tr>
<tr><td>2</td><td>$100 to $17F</td><td> </td></tr>
<tr><td>3</td><td>$180 to $1FF</td><td> </td></tr>
<tr><td>4</td><td>$200 to $27F</td><td>OPUS</td></tr>
<tr><td>5</td><td>$280 to $2FF</td><td>OPUS</td></tr>
<tr><td>6</td><td>$300 to $37F</td><td>Program registers P000 to P127</td></tr>
<tr><td>7</td><td>$380 to $3BF</td><td>Variable registers V00 to V63</td></tr>
<tr><td> </td><td>$3C0 to $3FF</td><td>OPUS</td></tr>
<tr><td>8</td><td>$400 to $47F</td><td>OPUS</td></tr>
<tr><td>9</td><td>$480 to $4FF</td><td>OPUS and global variables for OPUS</td></tr>
<tr><td>10</td><td>$500 to $57F</td><td>OPUS</td></tr>
<tr><td>11</td><td>$580 to $5FF</td><td>OPUS</td></tr>
<tr><td>12</td><td>$600 to $67F</td><td>OPUS</td></tr>
<tr><td>13</td><td>$680 to $6FF</td><td>OPUS</td></tr>
<tr><td>14</td><td>$700 to $77F</td><td>OPUS</td></tr>
<tr><td>15</td><td>$780 to $7FF</td><td>OPUS</td></tr>
<tr><td>16</td><td>$800 to $87F</td><td>Free space</td></tr>
<tr><td>17</td><td>$880 to $8FF</td><td>Free space</td></tr>
<tr><td>18</td><td>$900 to $97F</td><td>Free space</td></tr>
<tr><td>19</td><td>$980 to $9FF</td><td>Free space</td></tr>
<tr><td>20</td><td>$A00 to $A7F</td><td>Free space</td></tr>
<tr><td>21</td><td>$A80 to $AFF</td><td>Free space</td></tr>
<tr><td>22</td><td>$B00 to $B7F</td><td>Free space</td></tr>
<tr><td>23</td><td>$B80 to $BFF</td><td>Free space</td></tr>
<tr><td>24</td><td>$C00 to $C7F</td><td>Free space</td></tr>
<tr><td>25</td><td>$C80 to $CFF</td><td>Free space</td></tr>
<tr><td>26</td><td>$D00 to $D7F</td><td>Free space</td></tr>
<tr><td>27</td><td>$D80 to $DFF</td><td>Free space</td></tr>
<tr><td>28</td><td>$E00 to $E7F</td><td>Free space</td></tr>
<tr><td>29</td><td>$E80 to $EFF</td><td>Free space</td></tr>
<tr><td>30</td><td>$F00 to $F7F</td><td>OPUS</td></tr>
<tr><td>31</td><td>$F80 to $FFF</td><td>OPUS</td></tr>
</table>

The OPUS tracks 10-15, 30, and 31 are read-only.  All others are read/write.

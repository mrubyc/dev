# PSoC5LP I2C class

## usage

```
i2c = I2C.new()

# write
i2c.write( i2c_address, device_register, data1, data2, ... )
i2c.write( i2c_address, device_register, "String")

# read
s = i2c.read( i2c_address, device_register, length )
```

## example

### ST micro LPS25H air pressure sensor.

http://www.st.com/ja/mems-and-sensors/lps25h.html

```
ADRS_LPS25H = 0x5c

def to_int16( b1, b2 )
  return (b1 << 8 | b2) - ((b1 & 0x80) << 9)
end

# initialize
i2c.write( ADRS_LPS25H, 0x20, 0x90 )

# read
s = i2c.read( ADRS_LPS25H, 0xa8, 5 )
atm = ((s[2].ord << 16) | (s[1].ord << 8) | (s[0].ord)).to_f / 4096
tmp = 42.5 + to_int16(s[4].ord, s[3].ord).to_f / 480
s2 = sprintf( "a:%.2f t:%.2f", atm, tmp )
```

### TI HDC1000 humidity sensor.

http://www.ti.com/product/HDC1000

```
ADRS_HDC1000 = 0x40

def to_uint16( b1, b2 )
  return (b1 << 8 | b2)
end

# initialize
i2c.write( ADRS_HDC1000, 0x02, 0x16, 0x00 )

# read
i2c.read( ADRS_HDC1000, 0x00 )
sleep( 0.007 )
s = i2c.read( ADRS_HDC1000, nil, 4 )
tmp = to_uint16( s[0].ord, s[1].ord ).to_f / 65536 * 165 - 40
hum = to_uint16( s[2].ord, s[3].ord ).to_f / 65536 * 100
s2 = sprintf( "t:%.2f h:%.0f", tmp, hum );
```

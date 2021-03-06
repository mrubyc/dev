# PSoC5LP I2C class

## Cauton
The specifications of read and write methods was changed since release 1.
Because to match the specifications with the (Sometimes used on Raspberry Pi) I2C library of CRuby.


## usage

```
i2c = I2C.new()

# write
i2c.write( i2c_address, data1, data2, ... )
i2c.write( i2c_address, "String")

# read
#  If param is specified, send it first.
#  Then read the number of bytes specified by read_length.
s = i2c.read( i2c_address, read_length, *param )
```

## example

### ST micro LPS25H air pressure sensor.

http://www.st.com/ja/mems-and-sensors/lps25h.html

```
ADRS_LPS25H = 0x5c

def to_int16( b1, b2 )
  return (b1 << 8 | b2) - ((b1 & 0x80) << 9)
end
def to_uint24( b1, b2, b3 )
  return b1 << 16 | b2 << 8 | b3
end


# initialize
i2c = I2C.new()
i2c.write( ADRS_LPS25H, 0x20, 0x90 )

# read
s = i2c.read( ADRS_LPS25H, 5, 0xa8 )
atm = to_uint24( s.getbyte(2), s.getbyte(1), s.getbyte(0) ).to_f / 4096
tmp = 42.5 + to_int16(s.getbyte(4), s.getbyte(3)).to_f / 480
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
i2c = I2C.new()
i2c.write( ADRS_HDC1000, 0x02, 0x16, 0x00 )

# read
i2c.read( ADRS_HDC1000, 0, 0x00, :NO_STOP )
sleep( 0.007 )
s = i2c.read( ADRS_HDC1000, 4 )

tmp = to_uint16( s.getbyte(0), s.getbyte(1) ).to_f / 65536 * 165 - 40
hum = to_uint16( s.getbyte(2), s.getbyte(3) ).to_f / 65536 * 100
s2 = sprintf( "t:%.2f h:%.0f", tmp, hum );
```

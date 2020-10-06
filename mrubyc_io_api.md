mruby/c IO API 仕様提案書

# GPIO

汎用デジタル入出力  
基本は1bitずつですが、機器によっては数ビットまとめて入出力ができる場合もあるでしょう。

## コンストラクタ
### GPIO.new( machine dependent parameters )

例 (mruby/c devkit 02)
```
gpio1 = GPIO.new( 1 )	# param: grove pin number.
```


## 入出力方向設定
### setmode( machine dependent parameters )

例 (mruby/c devkit 02)
```
gpio1.setmode( GPIO::IN )	# GPIO::IN or GPIO::OUT
```

（参考）機器によってはコンストラクタの引数で入出力の設定も併せて行い、実行途中では変更できないという方法が良いでしょう。


## 出力
### write( value )

例 (mruby/c devkit 02)
```
gpio1.write( 1 )
```

## 入力
### read() -> Integer

例 (mruby/c devkit 02)
```
val = gpio1.read()			# val = 0 or 1
```



# ADC

電圧入力  
汎用の電圧入力です。入力範囲などは、機器によって異なります。

## コンストラクタ
### ADC.new( machine dependent parameters )

例 (mruby/c devkit 02)
```
adc1 = ADC.new( 1 )	# param: grove pin number.
```

## 入力
### read() -> Float

例 (mruby/c devkit 02)
```
val = adc1.read()		# val = 0.0 - 2.048
```



# シリアル通信 UART
UARTシリアルインターフェースを扱います。

## コンストラクタ
### UART.new( machine dependent parameters )

例 (mruby/c devkit 02)
```
uart1 = UART.new( 1 )	# param: grove pin number.
```

## 出力
### write( string )
指定された文字列を出力します。

例 (mruby/c devkit 02)
```
uart1.write("Output string¥r¥n")
```

## 入力
### read( n_bytes ) -> String, Nil
指定されたバイト数のデータを読み込みます。指定されたバイト数のデータが到着していない場合、nilを返します。

例 (mruby/c devkit 02)
```
val = uart1.read( 10 )
```

（参考）通常、データが到着していなければブロックする方式が一般的ですが、mruby/cで作るアプリケーションの場合、あえてブロックしない方式としたほうがプログラミングしやすかったので、そのような仕様になっています。


### read_nonblock( maxlen ) -> String
指定されたバイト数のデータを読み込みます。指定されたバイト数のデータが到着していない場合、到着している分のデータを返します。

例 (mruby/c devkit 02)
```
val = uart1.read_nonblock( 1024 )
```

### gets()
文字列を一行読み込みます。実際には受信キュー内の "\n" までのバイト列を返します。
受信キューに "\n" が無い場合、nilを返します。

例 (mruby/c devkit 02)
```
val = uart1.gets()
```

（参考）受信キューより長い文字列を受信した場合、gets()では処理できません。その場合、受信キューを大きくしたファームウェアを用意する必要があります。


## その他
### clear_tx_buffer()
読み込みバッファをクリアします。

例 (mruby/c devkit 02)
```
uart1.clear_tx_buffer()
```

### clear_rx_buffer()
書き込みバッファをクリアします。

例 (mruby/c devkit 02)
```
uart1.clear_rx_buffer()
```



# シリアル通信 I2C
I2Cシリアルインターフェースを扱います。

## コンストラクタ
### I2C.new( machine dependent parameters )

例 (mruby/c devkit 02)
```
i2c = I2C.new()
```

## 出力
### write( i2c_adrs_7, data1, data2,... )
指定されたバイトを順次出力します。

例 (mruby/c devkit 02)
```
i2c.write( ADRS, 0x02, 0x16, 0x00 )
```

### write( i2c_adrs_7, "string" )
指定された文字列を出力します。

例 (mruby/c devkit 02)
```
i2c.write( ADRS, "string" )
```

## 入力
### read( i2c_adrs_7, read_bytes, *params ) -> String
指定されたバイト数のデータを読み込みます。指定されたバイト数のデータが到着していない場合、ブロックします。  
paramsを指定することで、read前にparamsを出力します。すなわち、以下のシーケンスとなります。

```
	(S) - ADRS7 (W)(A) - [params ...] - (Sr) - ADRS7 (R)(A) - data_1 (A)... data_n (A|N) - (P)
	S : Start condition	P : Stop condition
	Sr: Repeated start condition
	A : Ack	N : Nack
```

例 (mruby/c devkit 02)
```
s = i2c.read( ADRS, 2, 0xfe )		# 0xfeレジスタから2バイト取得
```


# シリアル通信 SPI
SPIシリアルインターフェースを扱います。

## コンストラクタ
### SPI.new( machine dependent parameters )

例 (mruby/c devkit 02)
```
spi = SPI.new()
```

## 出力
### write( data1, data2,... )
指定されたバイトを順次出力します。

例 (mruby/c devkit 02)
```
spi.write( 0x02, 0x16, 0x00 )
```

### write( "string" )
指定された文字列を出力します。

例 (mruby/c devkit 02)
```
spi.write( "string" )
```

## 入力
### read( read_bytes )  -> String
指定されたバイト数のデータを読み込みます。

#### 戻り値

例 (mruby/c devkit 02)
```
s = spi.read( 2 )
```

## 汎用転送
### transfer( [d1, d2,...], recv_size ) -> String
d1,d2...を送信し、その後recv_size分の 0x00 を送信します。  
戻り値は、受信した長さ recv_size バイトの文字列となります。



# パルス幅変調 PWM
パルス幅変調(Pulse Width Modulation)を扱います。
周波数を指定する方法と、周期を指定する方法があります。

## コンストラクタ
### PWM.new( machine dependent parameters )

例 (mruby/c devkit 02)
```
pwm = PWM.new()
```

## 周波数を指定する方法

### frequency( n )
指定した周波数の信号を出力します。最大値は利用する機器に依存します。0を指定すると出力を停止します。

### duty( n )
デューティー比を、0（全オフ）から1023（全オン）までで指定します。

例 (mruby/c devkit 02)
```
pwm.frequency( 440 )   # 440Hz
pwm.duty( 512 )        # 50%
```

## 周期を指定する方法

### period_us( n )
周期を指定して信号を出力します。単位はマイクロ秒 (uS) です。
最大値は利用する機器に依存します。0を指定すると出力を停止します。

例 (mruby/c devkit 02)
```
pwm.period_us( 2273 )   # 440Hz
pwm.duty( 512 )         # 50%
```

（参考）機器によっては、上記のパラメータ設計では解像度が不足するかもしれません。その場合、浮動小数点でパラメータを与える事も考慮すべきだと考えます。

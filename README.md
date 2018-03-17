net_stream  
----------  
A PHP extension. Pack data into binary string. Unpack data from binary string.  
  
Building  
--------  
    $ phpize  
    $ ./configure --with-php-config=php-config --enable-net_stream  
    $ make  
    $ make install  
Add the following lines to your php.ini:  
    extension=net_stream.so  
  
Dependencies  
------------  
* Supports PHP 5.2 – 5.6, 7.0 – 7.2.  
* Requires ZLIB.  
  
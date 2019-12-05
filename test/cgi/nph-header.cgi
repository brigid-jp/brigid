#! /usr/bin/env lua

-- Copyright (c) 2019 <dev@brigid.jp>
-- This software is released under the MIT License.
-- https://opensource.org/licenses/mit-license.php

io.write(([[
HTTP/1.1 200 OK
Content-Type: text/plain; charset=UTF-8
Content-Length: 3
Connection: close
X-Test1:foo bar
X-Test2: foo bar 
X-Test3:  foo  bar  
X-Test4: foo
	bar
X-Test5: foo 
 
  bar
X-Test6:	 	 foo bar 	 	
x-test7: foo bar
X-tEsT8: foo bar

]]):gsub("\n", "\r\n"), "ok\n")

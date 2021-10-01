# Copyright (c) 2021 <dev@brigid.jp>
# This software is released under the MIT License.
# https://opensource.org/licenses/mit-license.php

TARGET = test_ragel.hpp

all: $(TARGET)

clean:
	rm -f $(TARGET)

test_ragel.hpp: test_ragel.rl
	ragel -G2 $< -o $@

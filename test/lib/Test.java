// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

public class Test {
  static {
    System.loadLibrary("test");
  }

  private native static void test();

  public static void main(String[] args) {
    test();
  }
}

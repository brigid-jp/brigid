// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

public class JavaTest {
  static {
    System.loadLibrary("javatest");
  }

  private native static int test(byte[] filename);

  public static void main(String[] args) throws Exception {
    System.exit(test(args[0].getBytes("UTF-8")));
  }
}

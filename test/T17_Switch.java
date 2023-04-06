class T17_Switch {

	static float f(int i) {
		switch(i) {
			case 0 : return 1.0F;
			case 1 : return 2.0F;
			case 2 : return 4.0F;
			case 3 :
			case 4 : return 8.0F;
			default : return 42.0F;
		}
	}

	public static void main(String... args) {
		if(f(0) != 1.0F) { System.exit(1); }
		if(f(1) != 2.0F) { System.exit(2); }
		if(f(2) != 4.0F) { System.exit(3); }
		if(f(3) != 8.0F) { System.exit(4); }
		if(f(4) != 8.0F) { System.exit(5); }
		if(f(-1) != 42.0F) { System.exit(42); }
	}

}
class T17_Switch {

	static float tableSwitch(int i) {
		switch(i) {
			case 0 : return 1.0F;
			case 1 : return 2.0F;
			case 2 : return 4.0F;
			case 3 :
			case 4 : return 8.0F;
			default : return 42.0F;
		}
	}

	static float lookupSwitch(int i) {
		switch(i) {
			case 00 : return 10.0F;
			case 10 : return 20.0F;
			case 20 : return 40.0F;
			case 30 :
			case 40 : return 80.0F;
			default : return 42.0F;
		}
	}

	public static void main(String... args) {
		if(tableSwitch(0) != 1.0F) { System.exit(1); }
		if(tableSwitch(1) != 2.0F) { System.exit(2); }
		if(tableSwitch(2) != 4.0F) { System.exit(3); }
		if(tableSwitch(3) != 8.0F) { System.exit(4); }
		if(tableSwitch(4) != 8.0F) { System.exit(5); }
		if(tableSwitch(-1) != 42.0F) { System.exit(6); }

		if(lookupSwitch(00) != 10.0F) { System.exit(11); }
		if(lookupSwitch(10) != 20.0F) { System.exit(12); }
		if(lookupSwitch(20) != 40.0F) { System.exit(13); }
		if(lookupSwitch(30) != 80.0F) { System.exit(14); }
		if(lookupSwitch(40) != 80.0F) { System.exit(15); }
		if(lookupSwitch(-1) != 42.0F) { System.exit(16); }
	}

}
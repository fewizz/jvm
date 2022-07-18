public class T02_SimpleLoop {
	
	public static void main(String... args) {
		int res = 0;

		for(int i = 0; i < 10; ++i) {
			++res;
		}
		
		if(res != 10) {
			System.exit(1);
		}

	}

}

public class T03_Throw {

	public static void main(String... args) {
		int i = 0;

		try {

			i = 1;
			if(i == 1) {
				throw new Throwable();
			}
			i = 2;

		} catch(Throwable t) {

			if(i != 1) {
				System.exit(1);
			}

			i = 3;
		}

		if(i != 3) {
			System.exit(1);
		}
	}
	
}

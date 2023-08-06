public class T04_CatchNPE {

	public static void main(String... args) {
		boolean catched = false;
		try {
			throw null;
		}
		catch(NullPointerException npe) {
			catched = true;
		}

		if(!catched) {
			System.exit(1);
		}
	}

}
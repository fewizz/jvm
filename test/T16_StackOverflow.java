public class T16_StackOverflow {

	static void recursive(int arg0, int arg1, int arg2, int arg3, int arg4) {
		int i = arg0;
		++i;
		recursive(i, arg1, arg2, arg3, arg4);
	}

	public static void main(String... args) {
		try {
			recursive(0, 1, 2, 3, 4);
		} catch(StackOverflowError err) {
			return;
		}
		System.exit(1);
	}

}
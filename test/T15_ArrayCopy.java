public class T15_ArrayCopy {

	public static void main(String... args) {
		char[] chars = new char[]{ 'a', 'b', 'c', 'd' };
		char[] copy = new char[4];
		System.arraycopy(chars, 0, copy, 0, 4);

		if(!(
			copy[0] == 'a' && copy[1] == 'b' && copy[2] == 'c' && copy[3] == 'd'
		)) {
			System.exit(1);
		}
	}

}
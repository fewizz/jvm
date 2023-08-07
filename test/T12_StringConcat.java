public class T12_StringConcat {

	public static void main(String... args) {
		String h = "Hello";
		String w = "world";
		String res = h + " " + w + "!";

		if(!res.equals("Hello world!")) {
			System.exit(1);
		}

		int value = 42;
		String withNumbers = res + value;
		if(!withNumbers.equals("Hello world!42")) {
			System.exit(1);
		}
	}

}
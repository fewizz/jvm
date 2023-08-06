public class StringBufferedHelloWorld {

	public static void main(String... args) {
		StringBuilder sb = new StringBuilder();
		sb.append("Hello");
		sb.append(' ');
		sb.append("world");
		sb.append('!');

		System.out.println(sb.toString());
	}

}
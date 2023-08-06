public class T06_ComponentType {

	public static void main(String... args) {
		Class<?> c = args.getClass().getComponentType();
		if(c != String.class) {
			System.exit(1);
		}

		c = int[].class.getComponentType();
		if(c != int.class) {
			System.exit(1);
		}
	}

}
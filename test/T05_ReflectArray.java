import java.lang.reflect.Array;

public class T05_ReflectArray {

	public static void main(String... agrs) {
		int[] array = (int[]) Array.newInstance(int.class, 2);
		if(array.length != 2) {
			System.exit(1);
		}
		if(array[0] != 0) {
			System.exit(2);
		}
		if(array[1] != 0) {
			System.exit(2);
		}
	}

}
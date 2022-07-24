package java.util;

public class Arrays {

	public static int hashCode(Object[] a) {
		if(a == null) {
			return 0;
		}

		int result = 1;

		for(Object o : a) {
			result += 31 * result + Objects.hashCode(o);
		}

		return result;
	}

	public static void sort(Object[] a) {
		throw new UnsupportedOperationException(); // TODO
	}

	public static <T> void sort(T[] a, Comparator<? super T> c) {
		throw new UnsupportedOperationException(); // TODO
	}

	//public static <T> T[] copyOf(T[] original, int newLength) {
	//
	//}

}
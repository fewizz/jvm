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

	@SafeVarargs
	public static <T> List<T> asList(T... a) {
		ArrayList<T> al = new ArrayList<T>(a.length);
		for(T t : a) {
			al.add(t);
		}
		return al;
	}

}
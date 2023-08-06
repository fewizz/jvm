package java.util;

public final class Objects {

	public static boolean equals(Object a, Object b) {
		if(a == b) {
			return true;
		}
		if(a != null) {
			return a.equals(b);
		}
		return false;
	}

	public static int hashCode(Object o) {
		return o != null ? o.hashCode() : 0;
	}

	public static int checkFromIndexSize(int fromIndex, int size, int length) {
		if(!(
			fromIndex >= 0 && size >= 0 && length >= 0 &&
			fromIndex + size <= length
		)) {
			throw new IndexOutOfBoundsException();
		}

		return fromIndex;
	}

}
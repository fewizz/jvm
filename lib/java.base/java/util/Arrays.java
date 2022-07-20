package java.util;

import java.util.Objects;

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

}
package java.lang;

public interface CharSequence {

	int length();

	char charAt(int index);

	default boolean isEmpty() {
		return length() == 0;
	}

	CharSequence subSequence(int start, int end);

	String toString();

	//default IntStream chars();

	//default IntStream codePoints();

	static int compare(CharSequence cs1, CharSequence cs2) {
		int len1 = cs1.length();
		int len2 = cs2.length();

		for(int x = 0; x < len1 && x < len2; ++x) {
			int diff = Character.compare(cs1.charAt(x), cs2.charAt(x));
			if(diff != 0) {
				return diff;
			}
		}

		return Integer.compare(len1, len2);
	}

}
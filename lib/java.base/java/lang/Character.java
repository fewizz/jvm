package java.lang;

import java.io.Serializable;

public final class Character implements Serializable, Comparable<Character> {

	private final char value_;

	public Character(char value) {
		this.value_ = value;
	}

	public char charValue() {
		return value_;
	}

	@Override
	public int compareTo(Character anotherCharacter) {
		return compare(this.value_, anotherCharacter.value_);
	}

	public static int compare(char x, char y) {
		return x == y ? 0 : x > y ? 1 : -1;
	}

}
package java.util;

import java.util.function.Consumer;

public interface Spliterator<T> {
	public static final int
		DISTINCT   = 0x00000001,
		SORTED     = 0x00000004,
		ORDERED    = 0x00000010,
		SIZED      = 0x00000040,
		NONNULL    = 0x00000100,
		IMMUTABLE  = 0x00000400,
		CONCURRENT = 0x00001000,
		SUBSIZED   = 0x00004000;


	boolean tryAdvance(Consumer<? super T> action);

	default void forEachRemaining(Consumer<? super T> action) {
		while(tryAdvance(action));
	}

	Spliterator<T> trySplit();

	long estimateSize();

	default long getExactSizeIfKnown() {
		return (characteristics() & SIZED) == 1 ? estimateSize() : -1L;
	}

	int characteristics();

	default boolean hasCharacteristics(int characteristics) {
		return (~characteristics() & characteristics()) == 0;
	}

	default Comparator<? super T> getComparator() {
		throw new IllegalStateException();
	}

}
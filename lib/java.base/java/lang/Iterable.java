package java.lang;

import java.util.function.Consumer;
import java.util.Iterator;

public interface Iterable<T> {

	Iterator<T> iterator();

	default void forEach(Consumer<? super T> action) {
		if(action == null) {
			throw new NullPointerException();
		}

		for (T t : this) {
			action.accept(t);
		}
	}

}
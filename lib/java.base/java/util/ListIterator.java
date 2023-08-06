package java.util;

public interface ListIterator<E> extends Iterator<E> {

	@Override
	boolean hasNext();

	@Override
	E next();

	boolean hasPrevious();

	E previous();

	int nextIndex();

	int previousIndex();

	@Override
	void remove();

	void set(E e);

	void add(E e);

}
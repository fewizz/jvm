package java.util;

public interface Collection<E> extends Iterable<E> {

	int size();

	boolean isEmpty();

	boolean contains(Object o);

	@Override
	Iterator<E> iterator();

	Object[] toArray();

	<T> T[] toArray(T[] a);

	boolean add(E e);

	boolean remove(Object o);

	boolean containsAll(Collection<?> c);

	boolean addAll(Collection<? extends E> c);

	boolean removeAll(Collection<?> c);

	void clear();

	@Override
	boolean equals(Object obj);

	@Override
	int hashCode();

}
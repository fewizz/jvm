package java.util;

public interface Set<E> extends Collection<E> {

	@Override
	int size();

	@Override
	boolean isEmpty();

	@Override
	boolean contains(Object o);

	@Override
	Iterator<E> iterator();

	@Override
	Object[] toArray();

	@Override
	<T> T[] toArray(T[] a);

	@Override
	boolean add(E e);

	@Override
	boolean remove(Object o);

	@Override
	boolean containsAll(Collection<?> c);

	@Override
	boolean addAll(Collection<? extends E> c);

	@Override
	boolean removeAll(Collection<?> c);

	@Override
	void clear();

	@Override
	boolean equals(Object o);

	@Override
	int hashCode();

}
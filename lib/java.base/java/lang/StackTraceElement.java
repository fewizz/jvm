package java.lang;

import java.io.Serializable;
import java.util.Objects;

public final class StackTraceElement implements Serializable {

	private final String
		classLoaderName_,
		moduleName_,
		moduleVersion_,
		declaringClass_,
		methodName_,
		fileName_;

	private final int lineNumber_;

	public StackTraceElement(
		String declaringClass, String methodName,
		String fileName, int lineNumber
	) {
		this.classLoaderName_ = null;
		this.moduleName_      = null;
		this.moduleVersion_   = null;
		this.declaringClass_  = declaringClass;
		this.methodName_      = methodName;
		this.fileName_        = fileName;
		this.lineNumber_      = lineNumber;
	}

	public StackTraceElement(
		String classLoaderName,
		String moduleName, String moduleVersion,
		String declaringClass, String methodName,
		String fileName, int lineNumber
	) {
		this.classLoaderName_ = classLoaderName;
		this.moduleName_      = moduleName;
		this.moduleVersion_   = moduleVersion;
		this.declaringClass_  = declaringClass;
		this.methodName_      = methodName;
		this.fileName_        = fileName;
		this.lineNumber_      = lineNumber;
	}

	public String getFileName() {
		return this.fileName_;
	}

	public int getLineNumber() {
		return this.lineNumber_;
	}

	public String getModuleName() {
		return this.moduleName_;
	}

	public String getModuleVersion() {
		return this.moduleVersion_;
	}

	public String getClassLoaderName() {
		return this.classLoaderName_;
	}

	public String getClassName() {
		return this.declaringClass_;
	}

	public String getMethodName() {
		return this.methodName_;
	}

	public boolean isNativeMethod() {
		return this.lineNumber_ == -2;
	}

	@Override
	public java.lang.String toString() {
		String loader = "";
		if(classLoaderName_ != null) {
			loader = classLoaderName_ + "/";
		}

		String mod = "/";
		if(moduleName_ != null && moduleVersion_ != null) {
			mod = moduleName_ + "@" + moduleVersion_ + "";
		}

		String where;

		if(fileName_ != null) {
			where = fileName_;
			if(lineNumber_ >= 0) {
				where += ":" + lineNumber_;
			}
		}
		else {
			where = isNativeMethod() ? "Native Method" : "Unknown Source";
		}

		String method = declaringClass_ + "." + methodName_;

		return loader + mod + method + "(" + where + ")";
	}

	@Override
	public boolean equals(Object obj) {
		if(obj == this) {
			return true;
		}

		if(!(obj instanceof StackTraceElement)) {
			return false;
		}

		StackTraceElement a = this;
		StackTraceElement b = (StackTraceElement) obj;

		return
			Objects.equals(a.getClassLoaderName(), b.getClassLoaderName()) &&
			Objects.equals(a.getModuleName(), b.getModuleName()) &&
			Objects.equals(a.getModuleVersion(), b.getModuleVersion()) &&
			Objects.equals(a.getClassName(), b.getClassName()) &&
			Objects.equals(a.getMethodName(), b.getMethodName()) &&
			Objects.equals(a.getFileName(), b.getFileName()) &&
			a.getLineNumber() == b.getLineNumber();
	}

	public int hashCode() {
		int result = 0;
		result = 31 * result + Objects.hashCode(classLoaderName_);
		result = 31 * result + Objects.hashCode(moduleName_);
		result = 31 * result + Objects.hashCode(moduleVersion_);
		result = 31 * declaringClass_.hashCode() + methodName_.hashCode();
		result = 31 * result + Objects.hashCode(fileName_);
		result = 31 * result + lineNumber_;
		return result;
	}

}
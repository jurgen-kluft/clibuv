package clibuv

import (
	"runtime"

	"github.com/jurgen-kluft/ccode/denv"
	ccore "github.com/jurgen-kluft/ccore/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "clibuv"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	ccorepkg := ccore.GetPackage()
	cunittestpkg := cunittest.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	mainpkg.AddPackage(ccorepkg)
	mainpkg.AddPackage(cunittestpkg)

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)

	// platform specific defines
	switch runtime.GOOS {
	case "darwin":
		mainlib.AddDefine("_DARWIN_UNLIMITED_SELECT=1", "_DARWIN_USE_64_BIT_INODE=1")
	case "windows":
		mainlib.AddDefine("WIN32_LEAN_AND_MEAN", "_WIN32_WINNT=0x0A00", "_CRT_DECLARE_NONSTDC_NAMES=0")
		mainlib.AddLib("psapi", "user32", "advapi32", "iphlpapi", "userenv", "ws2_32", "dbghelp", "ole32", "shell32")
	}

	// test library
	testlib := denv.SetupCppTestLibProject(mainpkg, name)
	testlib.AddDependencies(ccorepkg.GetTestLib())
	testlib.AddDependencies(cunittestpkg.GetTestLib())
	// platform specific defines
	switch runtime.GOOS {
	case "darwin":
		testlib.AddDefine("_DARWIN_UNLIMITED_SELECT=1", "_DARWIN_USE_64_BIT_INODE=1")
	case "windows":
		testlib.AddDefine("WIN32_LEAN_AND_MEAN", "_WIN32_WINNT=0x0A00", "_CRT_DECLARE_NONSTDC_NAMES=0")
		testlib.AddLib("psapi", "user32", "advapi32", "iphlpapi", "userenv", "ws2_32", "dbghelp", "ole32", "shell32")
	}

	// unittest project
	maintest := denv.SetupCppTestProject(mainpkg, name)
	maintest.AddDependencies(cunittestpkg.GetMainLib())
	maintest.AddDependency(testlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddTestLib(testlib)
	mainpkg.AddUnittest(maintest)

	return mainpkg
}

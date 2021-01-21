from setuptools import setup, find_packages, Command
from setuptools.command.install import install
import distutils.command.install as orig
from distutils.command.build import build
import subprocess
import inspect
import pathlib
from glob import glob

# Man this took a while. I want to run make to build the Distortion Correction code automatically. So I've made a custom build class which runs "build" and then runs make
# However, this creates all the files we need but they're in a temporary directory! They're not copied into the install directory 
# I'm therefore calling them "data" and telling setuptools to move them
# BUT: by default, data isn't moved to the same location as the install files, for some reason. We therefore use a hack to get the location of the installation files (which is probably something like lib/python3.7/site-packages/)
# This is a horrible hack... And 100% shouldn't be necessary. There's a "package_data" option in setuptools for just this kind of circumstance. But I couldn't get it to work- it doesn't seem to do what it's supposed to. This is a common question online and doesn't seem to be well documented __at all__
import pip , os
installation_location = os.path.split(os.path.split(pip.__file__)[0])[0]
print(f"installation_location: {installation_location}")

class CustomBuild(build):

  def run(self):
        # run original build code
        build.run(self)


        def compile():
          subprocess.call(['make', 'all_clean', '-C', 'hop/distortion_correction/HectorTranslationSoftware/Code'])
          subprocess.call(['make', '-C', 'hop/distortion_correction/HectorTranslationSoftware/Code'])

        self.execute(compile, [], 'Compiling Distortion Correction binary')




# The directory containing this file
HERE = pathlib.Path(__file__).parent

# The text of the README file
README = (HERE / "README.md").read_text()


data_files = [(f'/{installation_location}/hop/distortion_correction/HectorTranslationSoftware/Code', ['hop/distortion_correction/HectorTranslationSoftware/Code/HectorConfigUtil'] + glob('hop/distortion_correction/HectorTranslationSoftware/Code/*.o')), 
      (f'/{installation_location}/hop/distortion_correction/HectorTranslationSoftware/Packages/Misc', glob('hop/distortion_correction/HectorTranslationSoftware/Packages/Misc/*.o')), 
      (f'/{installation_location}/hop/distortion_correction/HectorTranslationSoftware/Packages/sds', glob('hop/distortion_correction/HectorTranslationSoftware/Packages/sds/*.o')),
      (f'/{installation_location}/hop/distortion_correction/HectorTranslationSoftware/Packages/DramaErs', glob('hop/distortion_correction/HectorTranslationSoftware/Packages/DramaErs/*.o')),
      (f'/{installation_location}/hop/distortion_correction/HectorTranslationSoftware/Packages/slalib_o', glob('hop/distortion_correction/HectorTranslationSoftware/Packages/slalib_o/*.o'))
      ]



setup(name='Hector-Observations-Pipeline',
      version='0.3.2',
      description='Hector Galaxy Survey Observations pipeline',
      long_description=README,
      long_description_content_type="text/markdown",
      author='Sam Vaughan',
      author_email='sam.vaughan@sydney.edu.au',
      url='https://github.com/samvaughan/Hector-Observations-Pipeline',
      license="GNU",
      classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8"],
      packages=find_packages(),
      include_package_data = True,
      data_files=data_files,
      python_requires='>=3',
      cmdclass={'build': CustomBuild}
     )


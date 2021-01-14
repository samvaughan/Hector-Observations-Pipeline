from setuptools import setup, find_packages, Command
import subprocess

class CustomInstall(Command):

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        subprocess.call(
            ['make', 
            '-C', 
            'hop/distortion_correction/HectorTranslationSoftware/Code']
        )


setup(name='Hector-Observations-Pipeline',
      version='0.0.3',
      description='Hector Galaxy Survey Observations pipeline',
      author='Sam Vaughan',
      author_email='sam.vaughan@sydney.edu.au',
      url='',
      packages=find_packages(),
      cmdclass={'customcommand': CustomInstall}
     )
import os
import shutil
import codecs
import json
import re


class Builder:
    """Traintastic manual builder base class"""

    def __init__(self, args):
        self._base_dir = args.base_dir
        self._output_dir = args.output_dir
        with codecs.open(os.path.join(self._base_dir, 'traintasticmanual.json'), 'r', 'utf-8') as json_file:
            self._json = json.load(json_file)
        self._language = args.language
        self._language_dir = os.path.join(args.base_dir, args.language)
        if not os.path.isdir(self._language_dir):
            raise Exception(self._language_dir + ' does not exist!')

    def _output_text_file(self, filename, text):
        """Write text file in output directory"""
        filename = os.path.join(self._output_dir, filename)
        os.makedirs(os.path.dirname(filename), mode=0o755, exist_ok=True)
        with codecs.open(filename, 'w', 'utf-8') as f:
            f.write(text)

    def _output_copy_files(self, filenames):
        for filename in filenames:
            os.makedirs(os.path.dirname(os.path.join(self._output_dir, filename)), mode=0o755, exist_ok=True)
            shutil.copyfile(os.path.join(self._base_dir, filename), os.path.join(self._output_dir, filename))

    def _output_copy_image(self, filename):
        image = re.sub(r'^.*/gfx/', '', filename)
        dst = os.path.join(self._output_dir, 'gfx', image)
        os.makedirs(os.path.dirname(dst), mode=0o755, exist_ok=True)
        shutil.copyfile(filename, dst)
        return './gfx/' + image
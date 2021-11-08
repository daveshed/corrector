import subprocess

my_process = subprocess.Popen(
    ['./build/src/spellchecker'],
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
    stderr=subprocess.STDOUT,
)

def check(word: str):
    my_process.stdin.write((word + '\n').encode())
    my_process.stdin.flush()
    to_parse = my_process.stdout.readline().decode()
    parsed = to_parse.strip(' ').split( )
    return parsed

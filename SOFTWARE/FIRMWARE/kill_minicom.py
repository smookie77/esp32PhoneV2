Import("env")

def kill_minicom(source, target, env):
    print("Killing minicom before upload...")
    env.Execute("killall -9 minicom || true")

env.AddPreAction("upload", kill_minicom)

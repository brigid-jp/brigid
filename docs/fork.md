## GitHub fork

https://help.github.com/ja/github/collaborating-with-issues-and-pull-requests/configuring-a-remote-for-a-fork

```
git remote add upstream https://github.com/love2d/love-android.git
git remote add upstream https://github.com/love2d/love.git
```


https://help.github.com/ja/github/collaborating-with-issues-and-pull-requests/syncing-a-fork

```
git fetch upstream
remote: Enumerating objects: 257, done.
remote: Counting objects: 100% (257/257), done.
remote: Compressing objects: 100% (39/39), done.
remote: Total 257 (delta 217), reused 252 (delta 212), pack-reused 0
Receiving objects: 100% (257/257), 1.11 MiB | 993.00 KiB/s, done.
Resolving deltas: 100% (217/217), completed with 202 local objects.
From https://github.com/love2d/love-android
 * [new branch]        0.10.x          -> upstream/0.10.x
 * [new branch]        0.11.x          -> upstream/0.11.x
 * [new branch]        http_downloader -> upstream/http_downloader
 * [new branch]        master          -> upstream/master
 * [new branch]        remove-libs     -> upstream/remove-libs
```

## edit

```
git clone brigid.github.com:brigid-jp/love.git
cd love
git config user.name dev
git config user.email dev@brigid.jp
git checkout 11.3
git checkout -b 11.3-develop
git-checkout-feature
# edit...
git checkout 11.3-develop
git merge --no-ff -m "作業ブランチをマージ。" feature
git branch -d feature
git-push-all-tags
```

## import

```
cd love/platform/xcode/ios
mv ~/Workspace/xcode/love-11.3-ios-libraries/* .
```

## check

```
diff -qr love love-11.3-ios-source
```

## build

```
open love/platform/xcode/love.xcodeproj
```

## sync

```
git checkout 11.3-develop
git stash
git checkout master
git-clean -f
rm -fr src/libraries/brigid
git merge upstream/master
```

minorの初回チェックアウトとマージ。

```
git checkout -b minor origin/minor
git merge upstream/minor
```

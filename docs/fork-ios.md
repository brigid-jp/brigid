## GitHub fork

https://help.github.com/ja/github/collaborating-with-issues-and-pull-requests/configuring-a-remote-for-a-fork

```
git remote add upstream https://github.com/love2d/love.git
```

```
git remote -v
origin	brigid.github.com:brigid-jp/love.git (fetch)
origin	brigid.github.com:brigid-jp/love.git (push)
upstream	https://github.com/love2d/love.git (fetch)
upstream	https://github.com/love2d/love.git (push)
```

https://help.github.com/ja/github/collaborating-with-issues-and-pull-requests/syncing-a-fork

```
git fetch upstream
remote: Enumerating objects: 4566, done.
remote: Counting objects: 100% (3120/3120), done.
remote: Compressing objects: 100% (495/495), done.
remote: Total 4566 (delta 2770), reused 2935 (delta 2613), pack-reused 1446
Receiving objects: 100% (4566/4566), 2.66 MiB | 6.22 MiB/s, done.
Resolving deltas: 100% (3608/3608), completed with 751 local objects.
From https://github.com/love2d/love
 * [new branch]        12.0-development -> upstream/12.0-development
 * [new branch]        master           -> upstream/master
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

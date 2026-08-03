# Use of AI in generating pull requests

The project accepts **AI-assisted** pull requests. 

However the project will not accept **AI-generated** pull requests where the pull request appears such that the developer only wrote a prompt and copy-pasted the output into pull request without doing anything more than that.
Such pull requests have very low value because the project maintainers have access to the same AI tools and can write prompts too. The problem with this approach is that it results in a lot of code which nobody understand how it works, making it difficult to fix bugs or incompatibilities with other environments.

The real effort, however, comes not from writing prompts. A proper AI-assisted pull request would do the following:

1. Summarize the problem in the prompt, and ask for solution;
2. Understand what has been proposed: do I understand what the change does and why it does it this way?
3. Review the proposed solution critically ("is #2 really needed? Would #5 break backward compatibility with KDE? can the change suggested in #8 be done using less code?")
4. Generate an updated solution taking the points from item 3 into account;
5. Review the proposed code -  does it do ONLY what it is supposed to do, or can the change have larger effects? Can those be avoided?
6. Test the code in multiple environments. Did it fix the problem it supposed to fix? Did it introduce other problems? "Adding support for X while breaking Y" is not a legitimate pull request unless you successfully argue support for Y should be dropped;
7. Document the 1-6 in pull request in form of summary. You should include the prompt, the rationale for the solution chosen, and the AI model which you used. This summary should be written by you, not AI-generated, because you would be expected to answer questions based on what you wrote. No need to use AI to polish your writing here; a pull request is not an English test, a rough 2-sentence summary is better than a 15-sentence essay saying the same thing.

Only at this moment such pull request would be taken seriously and reviewed. The real value here comes from steps 2-7, and this is what makes it AI-assisted but allows you to claim authorship on it.

If your role is limited to step 1 and you expect the maintainers to do steps 2-8, such requests will be rejected (and your authorship of such request would be questionable at best but likely non-existent).

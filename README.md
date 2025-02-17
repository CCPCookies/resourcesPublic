# carbon-template
GitHub Template Repository for Carbon C++ projects

Remember to rename everything!


# CI Pipeline Documentation

This pipeline automates handling open pull requests (PRs) when new commits are pushed to the repository.

---

## **How It Works**

1. **Trigger on Commit**  
   - The pipeline runs automatically every time a commit is pushed to any branch.

2. **Check for Open PRs**  
   - It checks if the committed branch is the **target branch** of any open PRs.

3. **Filter PRs**  
   - The pipeline identifies and sorts PRs into two categories:
     - **Domestic PRs**: Source branch belongs to the same repository.
     - **International PRs**: Source branch comes from a forked repository.

4. **Dynamic Builds**  
   - For each PR, the pipeline creates a separate build step using GitHub Actions' **matrix logic** to handle PRs in parallel.

5. **Merge Logic**  
   - Merges the latest changes from the **target branch** (committed branch) into the **source branch** of each PR.
   - Pushes the updated source branch back:
     - Domestic PRs: Push to the same repo.
     - International PRs: Push to the forked repo.

6. **Conflict Handling**  
   - If there are merge conflicts that require manual resolution, the specific PR build step fails.

---

## **Notes**

- The pipeline is dynamic and scales with the number of open PRs.
- Conflicts must be resolved manually, and affected builds will display the failure in the CI logs.
- Parallel builds ensure efficient processing of multiple PRs.

---

This setup ensures all PRs remain up-to-date with the latest changes from the target branch.



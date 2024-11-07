import lib280.exception.ContainerEmpty280Exception;
import lib280.tree.BinaryNode280;
import lib280.tree.LinkedSimpleTree280;
import lib280.tree.OrderedSimpleTree280;
import lib280.base.Dispenser280;
import lib280.base.Searchable280;
import lib280.exception.ContainerFull280Exception;
import lib280.exception.NoCurrentItem280Exception;

public class AVLTree280<I extends Comparable<? super I>> extends OrderedSimpleTree280<I>
        implements Dispenser280<I>, Searchable280<I> {

    /** Root of the tree */
    protected AVLBinaryNode280<I> rootNode;

    /** Current node */
    protected AVLBinaryNode280<I> cur;

    /** Parent node */
    protected AVLBinaryNode280<I> parent;

    /**	Do searches continue?. */
    protected boolean searchesContinue = false;

    /** Create an empty AVL tree */
    public AVLTree280() {
        super();
    }

    /** Create a new node of type AVLBinaryNode280
     * @param item    The item to be placed in the new node
     * @return the node
     */
    protected AVLBinaryNode280<I> createNewNode(I item)
    {
        return new AVLBinaryNode280<I>(item);
    }

    /**
     * Obtains the node at the root
     * @return the root node
     */
    protected AVLBinaryNode280<I> rootNode()
    {
        return rootNode;
    }

    /**
     * Obtains the item at the root
     * @return the item at the root
     * @precon !isEmpty()
     * @throws ContainerEmpty280Exception
     */
    public I rootItem() throws ContainerEmpty280Exception
    {
        if (isEmpty())
            throw new ContainerEmpty280Exception("Cannot access the root of an empty lib280.tree.");

        return rootNode.item();
    }

    /**
     * Obtains the left subtree of the root
     * @return the left subtree of the root
     * @precon !isEmpty()
     * @throws ContainerEmpty280Exception
     */
    public AVLTree280<I> rootLeftSubtree() throws ContainerEmpty280Exception
    {
        if (isEmpty())
            throw new ContainerEmpty280Exception("Cannot return a subtree of an empty lib280.tree.");

        AVLTree280<I> result = this.clone();
        result.clear();
        result.setRootNode(rootNode.leftNode);
        return result;
    }

    /**
     * Obtains the right subtree of the root
     * @return the right subtree of the root
     * @precon !isEmpty()
     * @throws ContainerEmpty280Exception
     */
    public AVLTree280<I> rootRightSubtree() throws ContainerEmpty280Exception
    {
        if (isEmpty())
            throw new ContainerEmpty280Exception("Cannot return a subtree of an empty lib280.tree.");

        AVLTree280<I> result = this.clone();
        result.clear();
        result.setRootNode(rootNode.rightNode);
        return result;
    }

    /**
     * Sets the root node to newNode
     * @param newNode
     */
    protected void setRootNode(AVLBinaryNode280<I> newNode)
    {
        rootNode = newNode;
    }

    /**	A shallow clone of this tree
     * @return a shallow clone of the tree
     */
    public AVLTree280<I> clone()
    {
        return (AVLTree280<I>) super.clone();
    }

    /**
     * Insert a node into the tree
     * @param data item to be inserted
     * @param R root node of the tree
     * @throws ContainerFull280Exception
     */
    public void insert(I data, AVLBinaryNode280<I> R) throws ContainerFull280Exception {
        if (this.isFull()) {
            throw new ContainerFull280Exception("Cannot insert item. Tree is full.");
        }
        // if the tree is empty, create a new node at the root
        if (this.isEmpty()) {
            this.setRootNode(createNewNode(data));
        }
        else {
            // if the data to be inserted is smaller than the data at the root
            if (data.compareTo(this.rootNode().item()) < 0) {
                // if the root's left child is empty, create one
                if (this.rootNode().leftNode() == null) {
                    this.rootNode().setLeftNode(createNewNode(data));
                } else { // move down the left subtree
                    this.rootLeftSubtree().insert(data, this.rootNode());
                }
                // recompute heights
                this.rootNode().setLeftHeight(Math.max(this.rootNode().leftNode().getLeftHeight(), this.rootNode().leftNode().getRightHeight()) + 1);
            } else {
                // if the root's right child is empty, create one
                if (this.rootNode().rightNode() == null) {
                    this.rootNode().setRightNode(createNewNode(data));
                } else { // move down the right subtree
                    this.rootRightSubtree().insert(data, this.rootNode());
                }
                // recompute heights
                this.rootNode().setRightHeight(Math.max(this.rootNode().rightNode().getLeftHeight(), this.rootNode().rightNode().getRightHeight()) + 1);
            }
            restoreAVLProperty(R); // restore the AVL property
        }
    }

    /**
     * Delete an item from the tree
     * @precon has(data)
     * @precon !isEmpty()
     * @throws NoCurrentItem280Exception
     */
    public void deleteItem(I data, AVLBinaryNode280<I> R) throws NoCurrentItem280Exception {
        if(!has(data)) {
            throw new NoCurrentItem280Exception("There is no item to delete.");
        }
        if (isEmpty()) {
            throw new NoCurrentItem280Exception("Tree is empty.");
        }

        if (data.compareTo(this.rootNode().item()) < 0) {
            // move down left subtree
            this.rootLeftSubtree().deleteItem(data, this.rootNode());
            // recompute heights
            if (this.rootNode().leftNode() == null) {
                this.rootNode().setLeftHeight(0);
            }
            else {
                this.rootNode().setLeftHeight(Math.max(this.rootNode().leftNode().getLeftHeight(), this.rootNode().leftNode().getRightHeight()) + 1);
            }
            // restore AVL property
            restoreAVLProperty(R);
        }
        else if (data.compareTo(this.rootNode().item()) > 0) {
            // move down right subtree
            this.rootRightSubtree().deleteItem(data, this.rootNode());
            // recompute heights
            if (this.rootNode().rightNode() == null) {
                this.rootNode().setRightHeight(0);
            }
            else {
                this.rootNode().setRightHeight(Math.max(this.rootNode().rightNode().getLeftHeight(), this.rootNode().rightNode().getRightHeight()) + 1);
            }
            // restore AVL property
            restoreAVLProperty(R);
        }
        else {
            boolean foundReplacement = false;
            AVLBinaryNode280<I> replaceNode = null;

            /*	Test if there is only one child, so it can replace the root. */
            if (this.rootNode().rightNode() == null)
            {
                replaceNode = this.rootNode().leftNode();
                foundReplacement = true;
            }
            else if (this.rootNode().leftNode() == null)
            {
                replaceNode = this.rootNode().rightNode();
                foundReplacement = true;
            }
            else
                foundReplacement = false;

            if (foundReplacement)
            {
                /*	Set R to refer to the replacement node. */
                if (R == null)
                    setRootNode(replaceNode);
                else if (R.leftNode() == this.rootNode())
                    R.setLeftNode(replaceNode);
                else
                    R.setRightNode(replaceNode);
                this.setRootNode(replaceNode);
            }
            else {
                AVLBinaryNode280<I> Cur = this.rootNode();
                while (Cur.leftNode() != null) {
                    Cur = Cur.leftNode();
                }
                AVLBinaryNode280<I> rootNodeCopy = this.rootNode();
                deleteItem(Cur.item(), Cur);
                rootNodeCopy.setItem(Cur.item());
            }
        }

    }

    /**
     * Check if the tree contains x
     * @param x item whose presence is to be determined
     * @return whether the tree contains x
     */
    public boolean has(I x) {
        // save cursor state and search restart state.
        AVLBinaryNode280<I> saveParent = parent;
        AVLBinaryNode280<I> saveCur = cur;
        boolean saveSearchesContinue = this.searchesContinue;

        // Always start at the root.
        this.parent = null;
        this.cur = this.rootNode;
        this.restartSearches();

        // Search
        this.search(x);
        boolean result = itemExists();

        // Restore cursor state and search restart state.
        this.parent = saveParent;
        this.cur = saveCur;
        this.searchesContinue = saveSearchesContinue;

        return result;
    }

    public int height(AVLBinaryNode280<I> R) {
        if (R == null) {
            return 0;
        }
        int rightHeight = height(R.leftNode);
        int leftHeight = height(R.rightNode);
        return Math.max(rightHeight, leftHeight) + 1;
    }

    /**
     * Perform a left rotation on the tree
     * @param R root node of the tree
     */
    private void leftRotation(AVLBinaryNode280<I> R) {
        AVLTree280<I> C = this.rootRightSubtree(); // right subtree
        AVLTree280<I> D = C.rootLeftSubtree(); // right subtree's left subtree

        // Recompute heights
        if (D.isEmpty()) {
            this.rootNode.setRightHeight(0);
        }
        else {
            this.rootNode.setRightHeight(Math.max(D.rootNode.getLeftHeight(), D.rootNode.getRightHeight()) + 1);
        }
        if (this.isEmpty()) {
            C.rootNode.setLeftHeight(0);
        }
        else {
            C.rootNode.setLeftHeight(Math.max(this.rootNode.getLeftHeight(), this.rootNode.getRightHeight()) + 1);
        }

        // Update R and perform rotation
        if (R != null) {
            if (R.leftNode == this.rootNode) {
                R.setLeftNode(C.rootNode);
            }
            else {
                R.setRightNode(C.rootNode);
            }
        }

        C.rootNode.setLeftNode(this.rootNode);
        this.rootNode.setRightNode(D.rootNode);
        this.setRootNode(C.rootNode);
    }

    /**
     * Perform a right rotation on the tree
     * @param R root node of the tree
     */
    private void rightRotation(AVLBinaryNode280<I> R) {
        AVLTree280<I> B = this.rootLeftSubtree(); // left subtree
        AVLTree280<I> E = B.rootRightSubtree(); // left subtree's right subtree

        // Recompute the heights
        if (E.isEmpty()) {
            this.rootNode.setLeftHeight(0);
        }
        else {
            this.rootNode.setLeftHeight(Math.max(E.rootNode.getLeftHeight(), E.rootNode.getRightHeight()) + 1);
        }
        if (this.isEmpty()) {
            B.rootNode.setRightHeight(0);
        }
        else {
            B.rootNode.setRightHeight(Math.max(this.rootNode.getLeftHeight(), this.rootNode.getRightHeight()) + 1);
        }

        // Update R and perform rotation
        if (R != null) {
            if (R.rightNode == this.rootNode) {
                R.setRightNode(B.rootNode);
            }
            else {
                R.setLeftNode(B.rootNode);
            }
        }

        B.rootNode.setRightNode(this.rootNode);
        this.rootNode.setLeftNode(E.rootNode);
        this.setRootNode(B.rootNode);
    }

    /**
     * Calculate the signed imbalance of a node
     * @param N node to check
     * @return the signed imbalance of the node
     */
    private int signed_imbalance(AVLBinaryNode280<I> N) {
        return N.getLeftHeight() - N.getRightHeight();
    }

    /**
     * Restore the AVL property for the tree
     * @param R root node of the tree
     */
    private void restoreAVLProperty(AVLBinaryNode280<I> R) {
        int imbalanceR = signed_imbalance(R);

        // there is no critical node
        if (Math.abs(imbalanceR) <= 1) {
            return;
        }

        // R is left heavy
        if (imbalanceR == 2) {
            // R's left child is left heavy
            if (signed_imbalance(R.leftNode) >= 0) {
                // LL imbalance
                rightRotation(R);
            }
            else {
                // LR imbalance
                leftRotation(R.leftNode);
                rightRotation(R);
            }
        }
        // R is right heavy
        else {
            // R's right child is right heavy
            if (signed_imbalance(R.rightNode) <= 0) {
                // RR imbalance
                leftRotation(R);
            }
            else {
                // RL imbalance
                rightRotation(R.rightNode);
                leftRotation(R);
            }
        }
    }

    /**	Form a string representation that includes level numbers.
     Analysis: Time = O(n), where n = number of items in the (sub)lib280.tree
     @param i the level for the root of this (sub)lib280.tree
     */
    protected String toStringByLevel(int i)
    {
        StringBuffer blanks = new StringBuffer((i - 1) * 5);
        for (int j = 0; j < i - 1; j++)
            blanks.append("     ");

        String result = new String();
        if (!isEmpty() && (!rootLeftSubtree().isEmpty() || !rootRightSubtree().isEmpty()))
            result += rootRightSubtree().toStringByLevel(i+1);

        result += "\n" + blanks + i + "|" ;
        if (isEmpty())
            result += "-";
        else
        {
            result += rootNode.getLeftHeight() + "|" + rootNode.getRightHeight() + ": " + rootItem();
            if (!rootLeftSubtree().isEmpty() || !rootRightSubtree().isEmpty())
                result += rootLeftSubtree().toStringByLevel(i+1);
        }
        return result;
    }

    /**	String representation of the lib280.tree, level by level. <br>
     Analysis: Time = O(n), where n = number of items in the lib280.tree
     */
    public String toStringByLevel()
    {
        return toStringByLevel(1);
    }

    /**	Is the lib280.tree empty?.
     Analysis: Time = O(1)  */
    public boolean isEmpty()
    {
        return rootNode == null;
    }

    /**	Go to item x, if it is in the lib280.tree.  If searchesContinue, continue in the right subtree. <br>
     Analysis : Time = O(h) worst case, where h = height of the lib280.tree */
    public void search(I x)
    {
        boolean found = false;
        if (!searchesContinue || above())
        {
            parent = null;
            cur = rootNode;
        }
        else if (!below())
        {
            parent = cur;
            cur = cur.rightNode();
        }
        while (!found && itemExists())
        {
            if (x.compareTo(item()) < 0)
            {
                parent = cur;
                cur = parent.leftNode();
            }
            else if (x.compareTo(item()) > 0)
            {
                parent = cur;
                cur = parent.rightNode();
            }
            else
                found = true;
        }
    }

    /**	Is there a current node?. <br>
     Analysis : Time = O(1) */
    public boolean itemExists()
    {
        return cur != null;
    }

    /**	Contents of the current node. <br>
     Analysis : Time = O(1)  <br>
     PRECONDITION: <br>
     <ul>
     itemExists()
     </ul> */
    public I item() throws NoCurrentItem280Exception
    {
        if (!itemExists())
            throw new NoCurrentItem280Exception("Cannot access item when it does not exist");

        return cur.item();
    }

    public static void main(String args[]) {
        AVLTree280<Integer> tree = new AVLTree280<>();
        AVLBinaryNode280<Integer> node = new AVLBinaryNode280<>(5);
        tree.insert(5, node);
        System.out.println();
        System.out.println("Inserting 5 into empty tree");
        System.out.println(tree.toStringByLevel());
        tree.insert(3, node);
        System.out.println();
        System.out.println("Inserting 3");
        System.out.println(tree.toStringByLevel());
        tree.insert(8, node);
        System.out.println();
        System.out.println("Inserting 8");
        System.out.println(tree.toStringByLevel());
        tree.insert(2, node);
        System.out.println();
        System.out.println("Inserting 2");
        System.out.println(tree.toStringByLevel());
        tree.insert(6, node);
        System.out.println();
        System.out.println("Inserting 6");
        System.out.println(tree.toStringByLevel());
        tree.insert(10, node);
        System.out.println();
        System.out.println("Inserting 10");
        System.out.println(tree.toStringByLevel());
        tree.insert(4, node);
        System.out.println();
        System.out.println("Inserting 4");
        System.out.println(tree.toStringByLevel());
        tree.insert(7, node);
        System.out.println();
        System.out.println("Inserting 7");
        System.out.println(tree.toStringByLevel());


        System.out.println();

        if (tree.has(4)) {
            System.out.println("Tree has 4");
        } else {
            System.out.println("Tree does not have 4");
        }
        System.out.println();
        if (tree.has(5)) {
            System.out.println("Tree has 5");
        } else {
            System.out.println("Tree does not have 5");
        }
        System.out.println();
        if (tree.has(12)) {
            System.out.println("Tree has 12");
        } else {
            System.out.println("Tree does not have 12");
        }


        System.out.println();

        tree.deleteItem(7, node);
        System.out.println();
        System.out.println("Deleting 7");
        System.out.println(tree.toStringByLevel());
        tree.deleteItem(3, node);
        System.out.println();
        System.out.println("Deleting 3");
        System.out.println(tree.toStringByLevel());
        tree.deleteItem(5, node);
        System.out.println();
        System.out.println("Deleting 5");
        System.out.println(tree.toStringByLevel());

        System.out.println();
        System.out.println("Deleting 15");
        try {
            tree.deleteItem(15, node);
        } catch(Exception e) {
            System.out.println("Tree does not have 15");
        }


        System.out.println();

        if (tree.has(8)) {
            System.out.println("Tree has 8");
        } else {
            System.out.println("Tree does not have 8");
        }
        System.out.println();
        if (tree.has(5)) {
            System.out.println("Tree has 5");
        } else {
            System.out.println("Tree does not have 5");
        }
        System.out.println();
        if (tree.has(12)) {
            System.out.println("Tree has 12");
        } else {
            System.out.println("Tree does not have 12");
        }
    }
}

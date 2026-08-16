## PART1:  RTOS链表（RTOS任务链表管理机制）

RTOS任务管理链表由三种结构体构成：

1. ListItem_t  普通链表节点

2. MiniListItem_t    尾哨兵节点

3. List_t 链表管理容器

### 1. **链表管理容器**

```
typedef struct xLIST
{

	完整性检查1;
    UBaseType_t uxNumberOfItems;  //链表中普通节点数量;不包含哨兵

    ListItem_t *pxIndex;          //current_node 游标,指向链表种的一个节点

	MiniListItem_t xListEnd;      //尾哨兵节点：提供操作入口

	完整性检查2;
} List_t;
```

### 2. **普通链表节点**

```
typedef struct

{

完整性检查1;

    TickType_t xItemValue;            //链表中节点的排序依据(例如:唤醒时间)

    struct ListItem *pxNext;          //双向循环链表中的next

    struct ListItem *pxPrevious;      //双向循环链表中的prev

    void *pvOwner;                    //指向具体任务的TCB

struct List *pxContainer;             //位于哪一个类型的链表

完整性检查2;

} ListItem_t;
```




### 3. **尾哨兵**
```
struct xMINI_LIST_ITEM

{

完整性检查1;

TickType_t xItemValue;      

//哨兵节点排序值，与普通节点相同,一般数值最”极端”，排在节点末尾

    struct xLIST_ITEM *pxNext;   //next

    struct xLIST_ITEM *pxPrevious;   //prev

};

typedef struct xMINI_LIST_ITEM MiniListItem_t;
```

### 注：完整性检查一般是用于内核调试 / RAM 越界破坏检测机制。都是一些固定值比对校验

```
configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES == 1  //开启校验config
```

## PART2:  内核链表核心操作函数

RTOS内核链表核心操作函数:

1. vListInitialise() —— 初始化一个链表容器

2. vListInitialiseItem() —— 初始化一个普通节点

3. vListInsertEnd() —— 插入到遍历位置附近，主要服务于就绪链表/轮转

4. vListInsert() —按 xItemValue 排序插入，主要服务于延时、超时等链表

5. uxListRemove() —— 从当前链表删除节点

### 1. **vListInitialise****初始化链表容器

```
//入参1：注册的初始容器起始地址
void vListInitialise( List_t * const pxList )
{
	/* 遍历游标先指向哨兵节点 */
    pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );   
    
	//哨兵值设为最大，保证永远排在最后
    pxList->xListEnd.xItemValue = portMAX_DELAY;

	//空链表：哨兵 next 指向自己,previous 指向自己
    pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
    pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );

	//当前没有真实节点
    pxList->uxNumberOfItems = ( UBaseType_t ) 0U;
}
```

### 2.vListInitialiseItem()初始化普通节点

```
//入参1：注册普通节点的起始地址
void vListInitialiseItem( ListItem_t * const pxItem ) 
{
    pxItem->pxContainer = NULL; //当前该节点还未挂入任何链表

	//完整性检查
    listSET_FIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE( pxItem );
    listSET_SECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE( pxItem );
}
```

### 3.vListInsertEnd()将节点插到当前游标位置前面

```
//入参1：要插入的目标链表
//入参2：要被插入的目标节点
void vListInsertEnd( List_t * const pxList,
                     ListItem_t * const pxNewListItem )
{
    ListItem_t * const pxIndex = pxList->pxIndex; //获取游标节点地址

	//正常插入操作
    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;

    pxIndex->pxPrevious->pxNext = pxNewListItem;
    pxIndex->pxPrevious = pxNewListItem;

    pxNewListItem->pxContainer = pxList; //将新节点归属到目标链表

    pxList->uxNumberOfItems++;  //目标链表的正常节点数++
}
```

### 4.vListInsert()将节点按链表排列顺序（xItemValue）插入

```
//入参1：要插入的目标链表
//入参2：要被插入的目标节点
void vListInsert( List_t * const pxList,
                  ListItem_t * const pxNewListItem )
{
    /* 用于遍历链表、寻找插入位置 */
    ListItem_t * pxIterator;

    /* 获取新节点的排序值 */
    const TickType_t xValueOfInsertion =
        pxNewListItem->xItemValue;


    /* 如果新节点排序值已经是最大值，
       不能使用下面的普通遍历，否则可能因为
       xListEnd 的值也是 portMAX_DELAY 而无法结束 */
    if( xValueOfInsertion == portMAX_DELAY )
    {
        /* 直接定位到最后一个真实节点 */
        pxIterator = pxList->xListEnd.pxPrevious;
    }
    else
    {
        /* 从尾哨兵开始向后遍历，
           找到最后一个 xItemValue <= 新节点值的节点 */
        for( pxIterator = (ListItem_t *)&(pxList->xListEnd);
             pxIterator->pxNext->xItemValue <= xValueOfInsertion;
             pxIterator = pxIterator->pxNext )
        {
            /* 只遍历，不执行其他操作 */
        }
    }


    /* 在 pxIterator 和它的 next 之间插入新节点 */

    pxNewListItem->pxNext = pxIterator->pxNext;

    pxNewListItem->pxNext->pxPrevious = pxNewListItem;

    pxNewListItem->pxPrevious = pxIterator;

    pxIterator->pxNext = pxNewListItem;


    /* 记录节点属于当前链表 */
    pxNewListItem->pxContainer = pxList;

    /* 链表节点数量 +1 */
    pxList->uxNumberOfItems++;
}
```

### 5.uxListRemove()删除节点
```
//入参1：需要删除的节点的地址
UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )
{
    List_t * const pxList = pxItemToRemove->pxContainer; //获取删除节点所在的链表

	//删除链表中指定节点的操作
    pxItemToRemove->pxNext->pxPrevious =
        pxItemToRemove->pxPrevious;

    pxItemToRemove->pxPrevious->pxNext =
        pxItemToRemove->pxNext;

	//若删除的节点恰好为遍历的游标节点
    if( pxList->pxIndex == pxItemToRemove )
    {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }

	//释放资源
    pxItemToRemove->pxContainer = NULL;

    pxList->uxNumberOfItems--;

    return pxList->uxNumberOfItems;
}
```

注：为什么删除后要把游标往前推而不是往后退：
```
	//若删除的节点恰好为遍历的游标节点
    if( pxList->pxIndex == pxItemToRemove )
    {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }
```
解：初始链表的游标指向尾节点END，第一次遍历任务A，游标指向任务A，然后获取任务A的TCB
因此可得：游标指向的地址所对应的任务已经被遍历过了。
因此当前： 
```
A ⇄ B ⇄ C
    ↑
 pxIndex
```

删掉B时需要回到A代表下一次才遍历到C，C任务还未被遍历执行


## PART3:内核链表常用宏

